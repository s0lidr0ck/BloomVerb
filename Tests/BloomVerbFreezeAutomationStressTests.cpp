#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <tuple>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 96;
constexpr int kNumChannels = 2;
constexpr int kInputBlocks = 340;
constexpr int kTailBlocks = 220;
constexpr float kTwoPi = 6.28318530717958647692f;

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }

    return true;
}

void fillStressInput(juce::AudioBuffer<float>& block, int blockIndex, float& phaseL, float& phaseR)
{
    const float incL = kTwoPi * (180.0f + static_cast<float>((blockIndex % 7) * 11)) / static_cast<float>(kSampleRate);
    const float incR = kTwoPi * (277.0f + static_cast<float>((blockIndex % 5) * 13)) / static_cast<float>(kSampleRate);

    for (int sample = 0; sample < block.getNumSamples(); ++sample)
    {
        const float burst = (blockIndex % 9 == 0 && sample < 8) ? 0.35f : 0.0f;
        block.setSample(0, sample, 0.18f * std::sin(phaseL) + burst);
        block.setSample(1, sample, 0.17f * std::sin(phaseR) - burst * 0.7f);
        phaseL += incL;
        phaseR += incR;
        if (phaseL > kTwoPi)
            phaseL -= kTwoPi;
        if (phaseR > kTwoPi)
            phaseR -= kTwoPi;
    }
}

void accumulateChecksum(const juce::AudioBuffer<float>& block, std::uint64_t& checksum)
{
    for (int ch = 0; ch < block.getNumChannels(); ++ch)
    {
        const float* data = block.getReadPointer(ch);
        for (int i = 0; i < block.getNumSamples(); ++i)
        {
            const auto bits = static_cast<std::uint32_t>(juce::ByteOrder::swapIfBigEndian(std::bit_cast<std::uint32_t>(data[i])));
            checksum ^= bits + 0x9e3779b97f4a7c15ull + (checksum << 6u) + (checksum >> 2u);
        }
    }
}
} // namespace

int main()
{
    bloomverb::RuntimeParameters params;
    params.type = 2;
    params.size = 0.71f;
    params.decaySeconds = 7.8f;
    params.preDelayMs = 32.0f;
    params.diffusion = 0.82f;
    params.damping = 0.38f;
    params.early = 0.42f;
    params.width = 1.18f;
    params.mix = 0.92f;
    params.motion = 0.0f;
    params.dynamic = 0.62f;
    params.harmonic = 0.28f;
    params.warp = 0.58f;
    params.swell = 0.30f;
    params.texture = 0.67f;
    params.modRateHz = 0.0f;
    params.modDepth = 0.0f;
    params.duckAmount = 0.34f;
    params.bloomAmount = 0.61f;
    params.distance = 0.44f;
    params.transientPreserve = 0.52f;

    auto runScenario = [&]()
    {
        bloomverb::BloomVerbEngine engine;
        engine.prepare(kSampleRate, kBlockSize, kNumChannels);

        juce::AudioBuffer<float> block(kNumChannels, kBlockSize);
        float phaseL = 0.0f;
        float phaseR = 0.0f;
        float peak = 0.0f;
        double tailEnergy = 0.0;
        int tailSamples = 0;
        bool finite = true;
        std::uint64_t checksum = 1469598103934665603ull;

        for (int blockIndex = 0; blockIndex < kInputBlocks; ++blockIndex)
        {
            fillStressInput(block, blockIndex, phaseL, phaseR);
            params.freeze = ((blockIndex / 2) % 2) == 0;
            engine.process(block, params);
            accumulateChecksum(block, checksum);

            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                const float* data = block.getReadPointer(ch);
                for (int i = 0; i < block.getNumSamples(); ++i)
                {
                    finite = finite && std::isfinite(data[i]);
                    peak = juce::jmax(peak, std::abs(data[i]));
                }
            }
        }

        params.freeze = false;
        for (int blockIndex = 0; blockIndex < kTailBlocks; ++blockIndex)
        {
            block.clear();
            engine.process(block, params);
            accumulateChecksum(block, checksum);

            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                const float* data = block.getReadPointer(ch);
                for (int i = 0; i < block.getNumSamples(); ++i)
                {
                    const float value = data[i];
                    finite = finite && std::isfinite(value);
                    peak = juce::jmax(peak, std::abs(value));

                    if (blockIndex >= kTailBlocks - 40)
                    {
                        tailEnergy += static_cast<double>(value) * static_cast<double>(value);
                        ++tailSamples;
                    }
                }
            }
        }

        return std::tuple<bool, float, double, std::uint64_t> {
            finite,
            peak,
            std::sqrt(tailEnergy / juce::jmax(1, tailSamples)),
            checksum
        };
    };

    const auto [finiteA, peakA, tailRmsA, checksumA] = runScenario();
    const auto [finiteB, peakB, tailRmsB, checksumB] = runScenario();

    bool ok = true;
    ok &= expect(finiteA && finiteB, "Freeze automation produced non-finite samples");
    ok &= expect(juce::jmax(peakA, peakB) < 8.0f, "Freeze automation exceeded peak guardrail");
    ok &= expect(juce::jmax(tailRmsA, tailRmsB) < 0.12, "Tail energy stayed too hot after freeze release");
    ok &= expect(std::abs(tailRmsA - tailRmsB) < 1.0e-8, "Freeze tail RMS replay drifted unexpectedly");
    ok &= expect(checksumA == checksumB, "Freeze automation deterministic replay mismatch");

    if (!ok)
        return 1;

    std::cout << "BloomVerbFreezeAutomationStressTests passed.\n";
    return 0;
}
