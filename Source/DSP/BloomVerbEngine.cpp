#include "BloomVerbEngine.h"

#include <cmath>

namespace
{
constexpr float twoPi = 6.28318530717958647692f;
}

namespace bloomverb
{
void BloomVerbEngine::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRateHz = juce::jmax(1.0, sampleRate);
    maxSamplesPerBlock = juce::jmax(1, maxBlockSize);
    channelCount = juce::jmax(1, juce::jmin(2, numChannels));

    delayBufferSize = static_cast<int>(std::ceil(sampleRateHz * 1.5));
    delayBufferSize = juce::jmax(delayBufferSize, maxSamplesPerBlock * 4);

    for (auto& channel : preDelayBuffer)
        channel.assign(static_cast<size_t>(delayBufferSize), 0.0f);
    for (auto& channel : earlyDelayBuffer)
        channel.assign(static_cast<size_t>(delayBufferSize), 0.0f);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRateHz;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxSamplesPerBlock);
    spec.numChannels = 1;

    for (auto& filter : lowCutFilters)
    {
        filter.prepare(spec);
        filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    }

    for (auto& filter : highCutFilters)
    {
        filter.prepare(spec);
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }

    reverb.reset();
    reset();
}

void BloomVerbEngine::reset()
{
    preDelayWritePos = 0;
    earlyDelayWritePos = 0;
    diffPrevIn = { 0.0f, 0.0f };
    diffPrevOut = { 0.0f, 0.0f };
    fastEnvelope = 0.0f;
    slowEnvelope = 0.0f;
    releaseState = 0.0f;
    lfoPhase = 0.0f;

    for (auto& channel : preDelayBuffer)
        std::fill(channel.begin(), channel.end(), 0.0f);
    for (auto& channel : earlyDelayBuffer)
        std::fill(channel.begin(), channel.end(), 0.0f);

    for (auto& filter : lowCutFilters)
        filter.reset();
    for (auto& filter : highCutFilters)
        filter.reset();
    reverb.reset();
}

BloomVerbEngine::TypeProfile BloomVerbEngine::getTypeProfile(int type)
{
    switch (type)
    {
        case 0: return { 0.85f, -0.10f, 0.70f, 0.70f, 0.85f, 0.80f, -0.10f }; // Plate
        case 1: return { 1.00f,  0.00f, 1.00f, 1.00f, 1.00f, 1.00f,  0.00f }; // Hall
        case 2: return { 0.75f, -0.05f, 0.80f, 1.35f, 0.80f, 0.70f, -0.15f }; // Room
        case 3: return { 1.35f,  0.10f, 1.30f, 0.65f, 1.40f, 1.20f,  0.10f }; // Cloud
        case 4: return { 1.20f,  0.02f, 1.15f, 0.80f, 1.25f, 1.00f,  0.05f }; // Bloom
        case 5: return { 1.00f, -0.05f, 0.95f, 0.90f, 1.10f, 0.90f,  0.35f }; // Grain
        case 6: return { 0.90f, -0.02f, 0.85f, 1.20f, 0.95f, 0.90f,  0.00f }; // Chamber
        case 7: return { 1.25f,  0.15f, 1.20f, 0.75f, 1.35f, 1.50f,  0.15f }; // Dream
        default: break;
    }

    return {};
}

float BloomVerbEngine::saturate(float x, float amount)
{
    if (amount <= 0.0001f)
        return x;

    const float drive = 1.0f + amount * 6.0f;
    const float normaliser = std::tanh(drive);
    return std::tanh(x * drive) / juce::jmax(0.0001f, normaliser);
}

void BloomVerbEngine::process(juce::AudioBuffer<float>& buffer, const RuntimeParameters& parameters)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = juce::jmin(2, buffer.getNumChannels());
    if (numChannels < 1 || numSamples <= 0)
        return;

    auto dry = buffer;
    const auto profile = getTypeProfile(parameters.type);

    const float fastCoeff = std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.007f));
    const float slowCoeff = std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.18f));

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = dry.getSample(0, i);
        const float inR = (numChannels > 1) ? dry.getSample(1, i) : inL;
        const float amplitude = 0.5f * (std::abs(inL) + std::abs(inR));

        fastEnvelope = fastCoeff * fastEnvelope + (1.0f - fastCoeff) * amplitude;
        slowEnvelope = slowCoeff * slowEnvelope + (1.0f - slowCoeff) * amplitude;

        const float delta = juce::jlimit(-1.0f, 1.0f, (slowEnvelope - fastEnvelope) * 6.0f);
        releaseState = juce::jlimit(0.0f, 1.0f, 0.5f + delta * 0.5f);
    }

    const float modRate = juce::jlimit(0.01f, 8.0f, parameters.modRateHz);
    const float modDepth = juce::jlimit(0.0f, 1.0f, parameters.modDepth) * juce::jlimit(0.0f, 1.0f, parameters.motion);
    const float phaseIncrement = twoPi * modRate / static_cast<float>(sampleRateHz);
    const float lfo = std::sin(lfoPhase);
    lfoPhase += phaseIncrement * static_cast<float>(numSamples);
    if (lfoPhase > twoPi)
        lfoPhase = std::fmod(lfoPhase, twoPi);

    const float distanceBias = parameters.distance * 0.45f;
    const float swellDelayScale = 1.0f + parameters.swell * 0.30f + distanceBias;
    const int preDelaySamples = juce::jlimit(0, delayBufferSize - 1,
                                             static_cast<int>((parameters.preDelayMs * profile.preDelayScale * swellDelayScale * sampleRateHz) / 1000.0));

    const float earlyGain = juce::jlimit(0.0f, 1.0f, parameters.early * profile.earlyScale);
    const float diffusionAmount = juce::jlimit(0.0f, 1.0f, parameters.diffusion + profile.textureBias * parameters.texture);
    const float allpassA = juce::jlimit(0.0f, 0.82f, 0.08f + (diffusionAmount * 0.72f));

    juce::AudioBuffer<float> wetInput(numChannels, numSamples);
    wetInput.clear();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float inputSample = buffer.getSample(ch, sample);

            preDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(preDelayWritePos)] = inputSample;
            const int preRead = (preDelayWritePos - preDelaySamples + delayBufferSize) % delayBufferSize;
            float delayed = preDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(preRead)];

            earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(earlyDelayWritePos)] = delayed;
            const int tapA = (earlyDelayWritePos - static_cast<int>(0.007f * sampleRateHz) + delayBufferSize) % delayBufferSize;
            const int tapB = (earlyDelayWritePos - static_cast<int>(0.013f * sampleRateHz) + delayBufferSize) % delayBufferSize;
            const int tapC = (earlyDelayWritePos - static_cast<int>(0.023f * sampleRateHz) + delayBufferSize) % delayBufferSize;

            const float earlyRef = 0.58f * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapA)]
                                 + 0.31f * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapB)]
                                 + 0.21f * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapC)];

            const float transientOpen = juce::jlimit(0.0f, 1.0f, (fastEnvelope - slowEnvelope) * 8.0f + 0.2f);
            const float swellSuppress = juce::jlimit(0.0f, 1.0f, 1.0f - parameters.swell * transientOpen * 0.9f);
            delayed *= swellSuppress;

            const float x = delayed + earlyRef * earlyGain;
            const float y = -allpassA * x + diffPrevIn[static_cast<size_t>(ch)] + allpassA * diffPrevOut[static_cast<size_t>(ch)];
            diffPrevIn[static_cast<size_t>(ch)] = x;
            diffPrevOut[static_cast<size_t>(ch)] = y;

            const float textured = juce::jmap(parameters.texture, x, y);
            wetInput.setSample(ch, sample, textured);
        }

        preDelayWritePos = (preDelayWritePos + 1) % delayBufferSize;
        earlyDelayWritePos = (earlyDelayWritePos + 1) % delayBufferSize;
    }

    juce::dsp::Reverb::Parameters reverbParams;
    const float motionInfluence = lfo * modDepth * profile.motionScale;
    reverbParams.roomSize = juce::jlimit(0.05f, 1.0f, 0.12f + parameters.size * 0.82f * profile.roomScale + motionInfluence * 0.12f);
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, parameters.damping + profile.dampingBias + parameters.warp * releaseState * 0.08f);
    reverbParams.width = juce::jlimit(0.0f, 1.0f, (parameters.width * 0.5f) + parameters.warp * releaseState * 0.12f);
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.freezeMode = parameters.freeze ? 1.0f : 0.0f;
    reverb.setParameters(reverbParams);

    juce::dsp::AudioBlock<float> wetBlock(wetInput);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
    reverb.process(wetContext);

    const float tone = juce::jlimit(-1.0f, 1.0f, parameters.tone);
    const float lowCutHz = juce::jlimit(20.0f, 20000.0f, parameters.lowCutHz * (1.0f - 0.15f * tone));
    const float highCutHz = juce::jlimit(20.0f, 20000.0f, parameters.highCutHz * (1.0f + 0.12f * tone));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        lowCutFilters[static_cast<size_t>(ch)].setCutoffFrequency(lowCutHz);
        highCutFilters[static_cast<size_t>(ch)].setCutoffFrequency(highCutHz);
    }

    const float bloom = juce::jlimit(0.0f, 1.0f, parameters.bloomAmount * releaseState);
    const float duck = juce::jlimit(0.0f, 0.92f, fastEnvelope * parameters.dynamic * parameters.duckAmount * 1.4f);
    const float warpGain = juce::jlimit(0.4f, 2.0f, 1.0f + parameters.warp * (releaseState - 0.3f) * 0.8f);
    const float transientOpen = juce::jlimit(0.0f, 1.0f, (fastEnvelope - slowEnvelope) * 8.0f + 0.2f);
    const float transientBlend = juce::jlimit(0.0f, 1.0f, parameters.transientPreserve * transientOpen);
    const float wetGain = juce::jlimit(0.0f, 2.0f, (1.0f - duck) * warpGain * (1.0f + bloom * 0.5f));
    const float dryMix = juce::jlimit(0.0f, 1.0f, 1.0f - parameters.mix + 0.35f * transientBlend);
    const float wetMix = juce::jlimit(0.0f, 1.0f, parameters.mix);
    const float outputGain = juce::Decibels::decibelsToGain(parameters.outputDb);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float wetL = wetInput.getSample(0, sample);
        float wetR = (numChannels > 1) ? wetInput.getSample(1, sample) : wetL;

        wetL = saturate(wetL, parameters.harmonic);
        wetR = saturate(wetR, parameters.harmonic);

        wetL = lowCutFilters[0].processSample(0, wetL);
        wetL = highCutFilters[0].processSample(0, wetL);

        if (numChannels > 1)
        {
            wetR = lowCutFilters[1].processSample(0, wetR);
            wetR = highCutFilters[1].processSample(0, wetR);
        }

        const float widthGrowth = 1.0f + (releaseState * parameters.dynamic * 0.35f * profile.widthGrowth);
        const float width = juce::jlimit(0.0f, 2.0f, parameters.width * widthGrowth);
        const float mid = 0.5f * (wetL + wetR);
        const float side = 0.5f * (wetL - wetR) * width;
        wetL = mid + side;
        wetR = mid - side;

        const float outL = (dry.getSample(0, sample) * dryMix + wetL * wetMix * wetGain) * outputGain;
        const float outR = (dry.getSample(numChannels > 1 ? 1 : 0, sample) * dryMix + wetR * wetMix * wetGain) * outputGain;

        buffer.setSample(0, sample, outL);
        if (numChannels > 1)
            buffer.setSample(1, sample, outR);
    }
}
} // namespace bloomverb
