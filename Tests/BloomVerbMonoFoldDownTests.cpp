#include <cmath>
#include <iostream>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr int kWarmupBlocks = 10;
constexpr int kMeasuredBlocks = 48;

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }

    return true;
}

void fillMonoInput(juce::AudioBuffer<float>& monoBuffer, float& phase)
{
    const float phaseIncA = kTwoPi * 173.0f / static_cast<float>(kSampleRate);
    const float phaseIncB = kTwoPi * 431.0f / static_cast<float>(kSampleRate);

    for (int sample = 0; sample < monoBuffer.getNumSamples(); ++sample)
    {
        const float tone = 0.18f * std::sin(phase)
                         + 0.08f * std::sin(phase * (phaseIncB / phaseIncA))
                         + ((sample % 67) == 0 ? 0.05f : 0.0f);
        monoBuffer.setSample(0, sample, tone);
        phase += phaseIncA;
        if (phase > kTwoPi)
            phase -= kTwoPi;
    }
}
} // namespace

int main()
{
    bloomverb::RuntimeParameters params;
    params.type = 1;
    params.size = 0.59f;
    params.decaySeconds = 4.6f;
    params.preDelayMs = 18.0f;
    params.diffusion = 0.73f;
    params.damping = 0.44f;
    params.early = 0.36f;
    params.width = 1.0f;
    params.mix = 0.72f;
    params.motion = 0.0f;
    params.harmonic = 0.22f;
    params.warp = 0.24f;
    params.swell = 0.18f;
    params.texture = 0.30f;
    params.modRateHz = 0.0f;
    params.modDepth = 0.0f;
    params.duckAmount = 0.25f;
    params.bloomAmount = 0.48f;
    params.distance = 0.36f;
    params.transientPreserve = 0.62f;

    bloomverb::BloomVerbEngine monoEngine;
    bloomverb::BloomVerbEngine stereoEngine;
    monoEngine.prepare(kSampleRate, kBlockSize, 1);
    stereoEngine.prepare(kSampleRate, kBlockSize, 2);

    juce::AudioBuffer<float> monoBlock(1, kBlockSize);
    juce::AudioBuffer<float> stereoBlock(2, kBlockSize);
    float phase = 0.0f;

    double monoEnergy = 0.0;
    double foldEnergy = 0.0;
    double diffEnergy = 0.0;
    float monoPeak = 0.0f;
    float foldPeak = 0.0f;
    bool finite = true;

    for (int blockIndex = 0; blockIndex < kWarmupBlocks + kMeasuredBlocks; ++blockIndex)
    {
        fillMonoInput(monoBlock, phase);
        stereoBlock.copyFrom(0, 0, monoBlock, 0, 0, kBlockSize);
        stereoBlock.copyFrom(1, 0, monoBlock, 0, 0, kBlockSize);

        monoEngine.process(monoBlock, params);
        stereoEngine.process(stereoBlock, params);

        if (blockIndex < kWarmupBlocks)
            continue;

        for (int sample = 0; sample < kBlockSize; ++sample)
        {
            const float monoValue = monoBlock.getSample(0, sample);
            const float foldedValue = 0.5f * (stereoBlock.getSample(0, sample) + stereoBlock.getSample(1, sample));
            const float diff = monoValue - foldedValue;

            finite = finite && std::isfinite(monoValue) && std::isfinite(foldedValue);
            monoPeak = juce::jmax(monoPeak, std::abs(monoValue));
            foldPeak = juce::jmax(foldPeak, std::abs(foldedValue));
            monoEnergy += static_cast<double>(monoValue) * static_cast<double>(monoValue);
            foldEnergy += static_cast<double>(foldedValue) * static_cast<double>(foldedValue);
            diffEnergy += static_cast<double>(diff) * static_cast<double>(diff);
        }
    }

    const double monoRms = std::sqrt(monoEnergy / static_cast<double>(kMeasuredBlocks * kBlockSize));
    const double foldRms = std::sqrt(foldEnergy / static_cast<double>(kMeasuredBlocks * kBlockSize));
    const double diffRms = std::sqrt(diffEnergy / static_cast<double>(kMeasuredBlocks * kBlockSize));
    const double rmsRatio = foldRms / juce::jmax(1.0e-12, monoRms);

    bool ok = true;
    ok &= expect(finite, "Mono and folded stereo outputs must remain finite");
    ok &= expect(monoPeak > 1.0e-3f && monoPeak < 8.0f, "Mono output peak was out of bounds");
    ok &= expect(foldPeak > 1.0e-3f && foldPeak < 8.0f, "Folded stereo peak was out of bounds");
    ok &= expect(rmsRatio > 0.75 && rmsRatio < 1.25, "Folded stereo RMS drifted too far from mono");
    ok &= expect(diffRms < 0.12, "Folded stereo diverged too far from mono render");

    if (!ok)
        return 1;

    std::cout << "BloomVerbMonoFoldDownTests passed.\n";
    return 0;
}
