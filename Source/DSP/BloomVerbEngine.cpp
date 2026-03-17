#include "BloomVerbEngine.h"
#include "BloomVerbSmoothers.h"
#include "BloomVerbTypeVoicing.h"
#include "CloudBloomDreamAlgorithm.h"
#include "FreezeController.h"
#include "GrainSpecialAlgorithm.h"
#include "HallRoomAlgorithm.h"
#include "PlateAlgorithm.h"
#include "ReverbFamilyMapping.h"

#include <cmath>
#include <fstream>

namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float minDecaySeconds = 0.10f;
constexpr float maxDecaySeconds = 30.0f;
constexpr float decaySmoothTimeSeconds = 0.060f;
constexpr float wetCalibrationGain = 32.0f;
}

namespace bloomverb
{
BloomVerbEngine::BloomVerbEngine()
    : freezeController(std::make_unique<FreezeController>()),
      parameterSmoothers(std::make_unique<RuntimeParameterSmoothers>())
{
    algorithms[static_cast<size_t>(ReverbFamily::Plate)] = std::make_unique<PlateAlgorithm>();
    algorithms[static_cast<size_t>(ReverbFamily::HallRoom)] = std::make_unique<HallRoomAlgorithm>();
    algorithms[static_cast<size_t>(ReverbFamily::CloudBloomDream)] = std::make_unique<CloudBloomDreamAlgorithm>();
    algorithms[static_cast<size_t>(ReverbFamily::GrainSpecial)] = std::make_unique<GrainSpecialAlgorithm>();
}

BloomVerbEngine::~BloomVerbEngine() = default;

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

    dryScratchBuffer.setSize(2, maxSamplesPerBlock, false, false, true);
    wetScratchBuffer.setSize(2, maxSamplesPerBlock, false, false, true);
    dryScratchBuffer.clear();
    wetScratchBuffer.clear();

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

    for (auto& alg : algorithms)
        if (alg)
            alg->prepare(sampleRateHz);
    freezeController->prepare(sampleRateHz);
    parameterSmoothers->prepare(sampleRateHz);
    parameterSmoothers->reset(RuntimeParameters {});
    reset();
}

void BloomVerbEngine::reset()
{
    activeType = -1;
    resetTypeDependentState();
    parameterSmoothers->reset(RuntimeParameters {});
}

void BloomVerbEngine::resetTypeDependentState()
{
    preDelayWritePos = 0;
    earlyDelayWritePos = 0;
    diffPrevIn = { 0.0f, 0.0f };
    diffPrevOut = { 0.0f, 0.0f };
    harmonicRecircState = { 0.0f, 0.0f };
    warpTailState = { 0.0f, 0.0f };
    fastEnvelope = 0.0f;
    slowEnvelope = 0.0f;
    releaseState = 0.0f;
    lfoPhase = 0.0f;
    motionState = 0.0f;
    warpContourState = 0.5f;
    decayTailControl = mapDecaySecondsToTailControl(2.80f);

    for (auto& channel : preDelayBuffer)
        std::fill(channel.begin(), channel.end(), 0.0f);
    for (auto& channel : earlyDelayBuffer)
        std::fill(channel.begin(), channel.end(), 0.0f);

    dryScratchBuffer.clear();
    wetScratchBuffer.clear();

    for (auto& filter : lowCutFilters)
        filter.reset();
    for (auto& filter : highCutFilters)
        filter.reset();

    for (auto& alg : algorithms)
        if (alg)
            alg->reset();
    freezeController->reset();
}

float BloomVerbEngine::saturate(float x, float amount)
{
    if (amount <= 0.0001f)
        return x;

    const float drive = 1.0f + amount * 6.0f;
    const float normaliser = std::tanh(drive);
    return std::tanh(x * drive) / juce::jmax(0.0001f, normaliser);
}

float BloomVerbEngine::mapDecaySecondsToTailControl(float decaySeconds)
{
    const float clampedSeconds = juce::jlimit(minDecaySeconds, maxDecaySeconds, decaySeconds);
    const float normalised = std::log(clampedSeconds / minDecaySeconds) / std::log(maxDecaySeconds / minDecaySeconds);
    return juce::jlimit(0.0f, 1.0f, normalised);
}

void BloomVerbEngine::process(juce::AudioBuffer<float>& buffer, const RuntimeParameters& parameters)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = juce::jmin(2, buffer.getNumChannels());
    if (numChannels < 1 || numSamples <= 0)
        return;

    if (delayBufferSize <= 0)
        return;

    if (numSamples > dryScratchBuffer.getNumSamples())
    {
        dryScratchBuffer.setSize(2, numSamples, false, false, true);
        wetScratchBuffer.setSize(2, numSamples, false, false, true);
    }

    dryScratchBuffer.clear();
    wetScratchBuffer.clear();

    for (int ch = 0; ch < numChannels; ++ch)
        dryScratchBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    if (numChannels == 1)
        dryScratchBuffer.copyFrom(1, 0, buffer, 0, 0, numSamples);

    parameterSmoothers->setTargets(parameters);
    freezeController->setTarget(parameters.freeze);
    if (parameters.type != activeType)
    {
        activeType = parameters.type;
        resetTypeDependentState();
    }

    const float fastCoeff = std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.007f));
    const float slowCoeff = std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.18f));
    const float decaySmoothCoeff = 1.0f - std::exp(-1.0f / static_cast<float>(sampleRateHz * decaySmoothTimeSeconds));
    const float motionSmoothCoeff = 1.0f - std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.012f));
    const float warpContourSmoothCoeff = 1.0f - std::exp(-1.0f / static_cast<float>(sampleRateHz * 0.100f));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto smoothed = parameterSmoothers->getNext();
        const auto& typeDescriptor = getTypeDescriptor(smoothed.type);
        const auto& profile = typeDescriptor.voicing;
        const auto freezeState = freezeController->getNextState();

        const float inL = dryScratchBuffer.getSample(0, sample);
        const float inR = dryScratchBuffer.getSample(numChannels > 1 ? 1 : 0, sample);
        const float amplitude = 0.5f * (std::abs(inL) + std::abs(inR));

        fastEnvelope = fastCoeff * fastEnvelope + (1.0f - fastCoeff) * amplitude;
        slowEnvelope = slowCoeff * slowEnvelope + (1.0f - slowCoeff) * amplitude;

        const float delta = juce::jlimit(-1.0f, 1.0f, (slowEnvelope - fastEnvelope) * 6.0f);
        releaseState = juce::jlimit(0.0f, 1.0f, 0.5f + delta * 0.5f);

        const float targetDecayTailControl = mapDecaySecondsToTailControl(smoothed.decaySeconds);
        decayTailControl += (targetDecayTailControl - decayTailControl) * decaySmoothCoeff;
        const float shapedDecayTail = std::pow(decayTailControl, 0.85f);
        const float decayRoomBias = juce::jmap(shapedDecayTail, 0.0f, 1.0f, -0.12f, 0.35f);
        const float decayDampingBias = juce::jmap(shapedDecayTail, 0.0f, 1.0f, 0.08f, -0.08f);
        const float decayWetContour = juce::jmap(shapedDecayTail, 0.0f, 1.0f, 0.82f, 1.55f);

        const float modRate = juce::jlimit(0.01f, 8.0f, smoothed.modRateHz * profile.modRateScale);
        const float modDepth = juce::jlimit(0.0f, 1.0f, smoothed.modDepth)
                             * juce::jlimit(0.0f, 1.0f, smoothed.motion) * 0.62f;
        const float phaseIncrement = twoPi * modRate / static_cast<float>(sampleRateHz);
        const float motionTarget = std::sin(lfoPhase) * modDepth * profile.motionScale;
        motionState += (motionTarget - motionState) * motionSmoothCoeff;
        lfoPhase += phaseIncrement;
        if (lfoPhase >= twoPi)
            lfoPhase -= twoPi;

        const float distanceNorm = juce::jlimit(0.0f, 1.0f, smoothed.distance);
        const float distanceBias = distanceNorm * 0.45f;
        const float distanceDirectScale = juce::jmap(distanceNorm, 1.0f, 0.42f);
        const float distanceWetLift = juce::jmap(distanceNorm, 1.0f, 1.75f);
        const float distanceHighCutScale = juce::jmap(distanceNorm, 1.0f, 0.38f);
        const float transientPreserve = juce::jlimit(0.0f, 1.0f, smoothed.transientPreserve);
        const float transientOpen = juce::jlimit(0.0f, 1.0f, (fastEnvelope - slowEnvelope) * 8.0f + 0.2f);
        const float swellDelayScale = 1.0f + smoothed.swell * 0.30f + distanceBias;
        const int preDelaySamples = juce::jlimit(0, delayBufferSize - 1,
                                                 static_cast<int>((smoothed.preDelayMs * profile.preDelayScale
                                                                   * swellDelayScale * sampleRateHz) / 1000.0));
        AlgorithmProcessContext algCtx;
        algCtx.smoothed = &smoothed;
        algCtx.typeDescriptor = &typeDescriptor;
        algCtx.freezeState = &freezeState;
        algCtx.decayTailControl = decayTailControl;
        algCtx.decayRoomBias = decayRoomBias;
        algCtx.decayDampingBias = decayDampingBias;
        algCtx.decayWetContour = decayWetContour;
        algCtx.distanceDirectScale = distanceDirectScale;
        algCtx.distanceWetLift = distanceWetLift;
        algCtx.distanceHighCutScale = distanceHighCutScale;
        algCtx.swellDelayScale = swellDelayScale;
        algCtx.preDelaySamples = preDelaySamples;
        algCtx.motionState = motionState;
        algCtx.releaseState = releaseState;
        algCtx.lfoPhase = lfoPhase;

        const float earlyGain = juce::jlimit(0.0f, 1.0f,
                                             smoothed.early * profile.earlyScale * (1.0f + transientPreserve * 0.12f));
        const float diffusionAmount = juce::jlimit(0.0f, 1.0f,
                                                   smoothed.diffusion
                                                   + profile.textureBias * smoothed.texture
                                                   + profile.diffusionBias);
        const float sampleAllpassA = juce::jlimit(0.0f, 0.94f, 0.10f + diffusionAmount * 0.80f + motionState * 0.18f);
        const float earlyTapBias = juce::jlimit(-1.0f, 1.0f, profile.earlyTapWeightBias);
        const float tapAWeight = juce::jlimit(0.20f, 0.80f, 0.58f + earlyTapBias * 0.18f);
        const float tapBWeight = juce::jlimit(0.10f, 0.60f, 0.31f - earlyTapBias * 0.04f);
        const float tapCWeight = juce::jlimit(0.05f, 0.50f, 1.0f - tapAWeight - tapBWeight);
        const float tapWeightNormaliser = 1.0f / juce::jmax(0.0001f, tapAWeight + tapBWeight + tapCWeight);
        const float harmonicTypeScale = juce::jlimit(0.72f, 1.30f, 1.0f + profile.harmonicTilt * 0.26f);

        float diffuseL = 0.0f;
        float diffuseR = 0.0f;

        for (int ch = 0; ch < 2; ++ch)
        {
            const float inputSample = dryScratchBuffer.getSample(ch, sample);

            preDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(preDelayWritePos)] = inputSample;
            const int preRead = (preDelayWritePos - preDelaySamples + delayBufferSize) % delayBufferSize;
            float delayed = preDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(preRead)];

            earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(earlyDelayWritePos)] = delayed;
            const int tapAOffset = juce::jlimit(1, delayBufferSize - 1,
                                                static_cast<int>(0.007f * profile.earlyTapScaleA * sampleRateHz));
            const int tapBOffset = juce::jlimit(1, delayBufferSize - 1,
                                                static_cast<int>(0.013f * profile.earlyTapScaleB * sampleRateHz));
            const int tapCOffset = juce::jlimit(1, delayBufferSize - 1,
                                                static_cast<int>(0.023f * profile.earlyTapScaleC * sampleRateHz));
            const int tapA = (earlyDelayWritePos - tapAOffset + delayBufferSize) % delayBufferSize;
            const int tapB = (earlyDelayWritePos - tapBOffset + delayBufferSize) % delayBufferSize;
            const int tapC = (earlyDelayWritePos - tapCOffset + delayBufferSize) % delayBufferSize;

            const float earlyRef = (tapAWeight * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapA)]
                                  + tapBWeight * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapB)]
                                  + tapCWeight * earlyDelayBuffer[static_cast<size_t>(ch)][static_cast<size_t>(tapC)])
                                 * tapWeightNormaliser;

            const float swellSuppression = smoothed.swell * transientOpen
                                         * juce::jlimit(0.18f, 0.90f, 0.90f - transientPreserve * 0.65f);
            delayed *= juce::jlimit(0.0f, 1.0f, 1.0f - swellSuppression);
            const float handoffBlend = juce::jlimit(0.0f, 1.0f,
                                                    0.32f + diffusionAmount * 0.42f
                                                        + smoothed.texture * 0.08f
                                                        + transientPreserve * 0.05f);
            const float directLeak = juce::jlimit(0.01f, 0.32f,
                                                  (0.16f - diffusionAmount * 0.10f
                                                      + smoothed.early * 0.06f
                                                      - smoothed.swell * 0.04f)
                                                  * distanceDirectScale);
            const float earlySeed = earlyRef * earlyGain;
            const float x = delayed * directLeak + earlySeed * (0.62f + handoffBlend * 0.42f);
            const float harmonicFeedAmount = juce::jlimit(0.0f, 1.0f,
                                                          smoothed.harmonic * harmonicTypeScale
                                                          * (0.16f + 0.24f * releaseState + 0.08f * std::abs(motionState)));
            const float shapedFeed = juce::jmap(harmonicFeedAmount * 0.45f, x, saturate(x, harmonicFeedAmount * 0.75f));
            const float y = -sampleAllpassA * shapedFeed
                          + diffPrevIn[static_cast<size_t>(ch)]
                          + sampleAllpassA * diffPrevOut[static_cast<size_t>(ch)];
            diffPrevIn[static_cast<size_t>(ch)] = shapedFeed;
            diffPrevOut[static_cast<size_t>(ch)] = y;
            harmonicRecircState[static_cast<size_t>(ch)] = harmonicRecircState[static_cast<size_t>(ch)] * 0.965f
                                                         + y * (0.025f + 0.035f * harmonicFeedAmount);

            const float recircColour = harmonicRecircState[static_cast<size_t>(ch)] * harmonicFeedAmount * 0.14f;
            const float textured = juce::jmap(juce::jlimit(0.0f, 1.0f, smoothed.texture + motionState * 0.06f),
                                              shapedFeed,
                                              y + recircColour);

            if (ch == 0)
                diffuseL = textured * freezeState.inputGain;
            else
                diffuseR = textured * freezeState.inputGain;
        }

        preDelayWritePos = (preDelayWritePos + 1) % delayBufferSize;
        earlyDelayWritePos = (earlyDelayWritePos + 1) % delayBufferSize;

        float wetL = 0.0f;
        float wetR = 0.0f;
        const auto family = getFamilyForType(smoothed.type);
        auto* alg = algorithms[static_cast<size_t>(family)].get();
        if (alg)
            alg->processSample(diffuseL, diffuseR, algCtx, wetL, wetR);

        wetL = juce::jmap(0.06f + diffusionAmount * 0.22f, wetL, wetL + diffuseL * 0.32f + harmonicRecircState[0] * 0.04f);
        wetR = juce::jmap(0.06f + diffusionAmount * 0.22f, wetR, wetR + diffuseR * 0.32f + harmonicRecircState[1] * 0.04f);

        const float tone = juce::jlimit(-1.0f, 1.0f, smoothed.tone);
        const float lowCutHz = juce::jlimit(20.0f, 20000.0f,
                                            smoothed.lowCutHz * (1.0f - 0.12f * tone) * profile.lowCutScale);
        const float highCutHz = juce::jlimit(20.0f, 20000.0f,
                                             smoothed.highCutHz * (1.0f + 0.10f * tone)
                                                 * profile.highCutScale * distanceHighCutScale);
        lowCutFilters[0].setCutoffFrequency(lowCutHz);
        lowCutFilters[1].setCutoffFrequency(lowCutHz);
        highCutFilters[0].setCutoffFrequency(highCutHz);
        highCutFilters[1].setCutoffFrequency(highCutHz);

        wetL = lowCutFilters[0].processSample(0, saturate(wetL, smoothed.harmonic * 0.18f));
        wetL = highCutFilters[0].processSample(0, wetL);
        wetR = lowCutFilters[1].processSample(0, saturate(wetR, smoothed.harmonic * 0.18f));
        wetR = highCutFilters[1].processSample(0, wetR);

        const float tailEnergy = 0.5f * (std::abs(wetL) + std::abs(wetR));
        const float contourTarget = juce::jlimit(0.0f, 1.0f,
                                                 0.20f + 0.56f * releaseState + (0.35f - tailEnergy) * 0.90f);
        warpContourState += (contourTarget - warpContourState) * warpContourSmoothCoeff;

        const float warpAmount = juce::jlimit(0.0f, 1.0f, smoothed.warp);
        const float warpTypeScale = juce::jlimit(0.78f, 1.28f, 1.0f + profile.warpTendency * 0.24f);
        const float densityAmount = warpAmount * warpTypeScale * (1.0f - warpContourState) * 0.10f;
        const float brightnessTilt = 1.0f + warpAmount * warpTypeScale * (warpContourState - 0.5f) * 0.15f;

        warpTailState[0] = warpTailState[0] * (0.95f + 0.02f * warpContourState) + wetL * (0.05f - 0.02f * warpContourState);
        warpTailState[1] = warpTailState[1] * (0.95f + 0.02f * warpContourState) + wetR * (0.05f - 0.02f * warpContourState);
        wetL = wetL * brightnessTilt + warpTailState[0] * densityAmount * 0.65f;
        wetR = wetR * brightnessTilt + warpTailState[1] * densityAmount * 0.65f;

        const float bloom = juce::jlimit(0.0f, 1.0f, smoothed.bloomAmount);
        const float duck = juce::jlimit(0.0f, 0.92f,
                                        fastEnvelope * smoothed.dynamic * smoothed.duckAmount
                                            * (1.4f - transientPreserve * 0.65f));
        const float transientBlend = juce::jlimit(0.0f, 1.0f, smoothed.transientPreserve * transientOpen);
        const float transientWetLift = 1.0f + transientBlend * 0.16f;
        const float tailWetContour = juce::jmax(decayWetContour, 1.0f - freezeState.blend * 0.15f);
        const float warpGain = juce::jlimit(0.72f, 1.38f, 1.0f + warpAmount * warpTypeScale * (releaseState - 0.25f) * 0.24f);
        const float bloomWetScale = 1.0f + bloom * 1.5f;
        const float wetGain = juce::jlimit(0.0f, 4.0f,
                                           (1.0f - duck) * warpGain * bloomWetScale
                                               * tailWetContour * transientWetLift * profile.wetGainBias
                                               * 1.35f * distanceWetLift);

        // #region agent log
        if (sample == 0)
        {
            static bool firstBloomLog = true;
            static float lastBloomAmount = -1.0f, lastReleaseState = -1.0f, lastBloom = -1.0f, lastWetGain = -1.0f;
            static float lastMotion = -1.0f, lastModDepth = -1.0f, lastMotionState = -1.0f, lastModRateHz = -1.0f;
            static float lastTexture = -1.0f, lastDiffusionAmount = -1.0f, lastTextureBias = -1.0f;
            static float lastSwell = -1.0f, lastSwellDelayScale = -1.0f, lastSwellSuppression = -1.0f;
            const float effectiveModDepth = juce::jlimit(0.0f, 1.0f, smoothed.modDepth)
                                         * juce::jlimit(0.0f, 1.0f, smoothed.motion) * 0.62f;
            const float swellSuppressionVal = smoothed.swell * transientOpen
                * juce::jlimit(0.18f, 0.90f, 0.90f - transientPreserve * 0.65f);
            const bool bloomChanged = firstBloomLog
                || std::abs(smoothed.bloomAmount - lastBloomAmount) > 0.02f
                || std::abs(releaseState - lastReleaseState) > 0.02f
                || std::abs(bloom - lastBloom) > 0.02f
                || std::abs(wetGain - lastWetGain) > 0.05f
                || std::abs(smoothed.motion - lastMotion) > 0.02f
                || std::abs(effectiveModDepth - lastModDepth) > 0.02f
                || std::abs(motionState - lastMotionState) > 0.02f
                || std::abs(smoothed.modRateHz - lastModRateHz) > 0.02f
                || std::abs(smoothed.texture - lastTexture) > 0.02f
                || std::abs(diffusionAmount - lastDiffusionAmount) > 0.02f
                || std::abs(profile.textureBias - lastTextureBias) > 0.01f
                || std::abs(smoothed.swell - lastSwell) > 0.02f
                || std::abs(swellDelayScale - lastSwellDelayScale) > 0.02f
                || std::abs(swellSuppressionVal - lastSwellSuppression) > 0.02f;
            if (bloomChanged)
            {
                std::ofstream out(R"(C:\PROJECTS\A18\BloomVerb\BloomVerb\debug-f8da05.log)", std::ios::app);
                if (out)
                {
                    const auto fmt = [](float v) { return juce::String(v, 4); };
                    out << "{\"sessionId\":\"f8da05\",\"runId\":\"bloom-debug\",\"hypothesisId\":\"H1-H5\","
                        << "\"location\":\"BloomVerbEngine.cpp:process\",\"message\":\"Bloom strip values\","
                        << "\"data\":{"
                        << "\"bloomAmount\":" << fmt(smoothed.bloomAmount)
                        << ",\"releaseState\":" << fmt(releaseState)
                        << ",\"bloom\":" << fmt(bloom)
                        << ",\"bloomWetScale\":" << fmt(bloomWetScale)
                        << ",\"wetGain\":" << fmt(wetGain)
                        << ",\"motion\":" << fmt(smoothed.motion)
                        << ",\"effectiveModDepth\":" << fmt(effectiveModDepth)
                        << ",\"modRateHz\":" << fmt(smoothed.modRateHz)
                        << ",\"motionState\":" << fmt(motionState)
                        << ",\"texture\":" << fmt(smoothed.texture)
                        << ",\"diffusionAmount\":" << fmt(diffusionAmount)
                        << ",\"textureBias\":" << fmt(profile.textureBias)
                        << ",\"swell\":" << fmt(smoothed.swell)
                        << ",\"swellDelayScale\":" << fmt(swellDelayScale)
                        << ",\"swellSuppression\":" << fmt(swellSuppressionVal)
                        << ",\"mix\":" << fmt(smoothed.mix)
                        << ",\"type\":" << smoothed.type
                        << "},\"timestamp\":" << juce::Time::currentTimeMillis() << "}\n";
                }
                firstBloomLog = false;
                lastBloomAmount = smoothed.bloomAmount;
                lastReleaseState = releaseState;
                lastBloom = bloom;
                lastWetGain = wetGain;
                lastMotion = smoothed.motion;
                lastModDepth = effectiveModDepth;
                lastMotionState = motionState;
                lastModRateHz = smoothed.modRateHz;
                lastTexture = smoothed.texture;
                lastDiffusionAmount = diffusionAmount;
                lastTextureBias = profile.textureBias;
                lastSwell = smoothed.swell;
                lastSwellDelayScale = swellDelayScale;
                lastSwellSuppression = swellSuppressionVal;
            }
        }
        // #endregion
        const float mix = juce::jlimit(0.0f, 1.0f, smoothed.mix);
        const float dryMix = std::sqrt(juce::jlimit(0.0f, 1.0f, 1.0f - mix));
        const float wetMix = std::sqrt(juce::jlimit(0.0f, 1.0f, mix + freezeState.blend * 0.05f));
        const float highMixWetLift = juce::jmap(juce::jlimit(0.0f, 1.0f, (mix - 0.90f) / 0.10f), 1.0f, 2.0f);
        const float outputGain = juce::Decibels::decibelsToGain(smoothed.outputDb);

        const float outL = (inL * dryMix + wetL * wetMix * wetGain * highMixWetLift * wetCalibrationGain) * outputGain;
        const float outR = (inR * dryMix + wetR * wetMix * wetGain * highMixWetLift * wetCalibrationGain) * outputGain;

        buffer.setSample(0, sample, outL);
        if (numChannels > 1)
            buffer.setSample(1, sample, outR);

        wetScratchBuffer.setSample(0, sample, wetL);
        wetScratchBuffer.setSample(1, sample, wetR);
    }
}
} // namespace bloomverb
