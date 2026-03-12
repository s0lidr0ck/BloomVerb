#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr int kNumChannels = 2;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr std::array<double, 3> kSampleRates { 44100.0, 48000.0, 96000.0 };
constexpr std::array<int, 4> kBlockSizes { 32, 64, 257, 1024 };
constexpr double kSignalSeconds = 0.18;
constexpr double kTailSeconds = 0.12;

struct Metrics
{
    bool finite = true;
    double energy = 0.0;
    float peak = 0.0f;
    int sampleCount = 0;
};

struct Result
{
    int type = 0;
    double sampleRate = 0.0;
    int blockSize = 0;
    Metrics metrics;
    double rms = 0.0;
    std::uint64_t checksum = 0;
};

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }

    return true;
}

float deterministicNoise(std::uint32_t index)
{
    std::uint32_t x = index * 747796405u + 2891336453u;
    x ^= x >> 16;
    x *= 2246822519u;
    return static_cast<float>(x & 0x00ffffffu) / static_cast<float>(0x00ffffffu) * 2.0f - 1.0f;
}

void fillInput(juce::AudioBuffer<float>& block, double sampleRate, int absoluteStart)
{
    const int numSamples = block.getNumSamples();
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const int absoluteSample = absoluteStart + sample;
        const float t = static_cast<float>(absoluteSample / sampleRate);
        const float impulse = (absoluteSample % juce::jmax(1, static_cast<int>(sampleRate * 0.043))) == 0 ? 0.8f : 0.0f;
        const float tone = 0.18f * std::sin(kTwoPi * 143.0f * t)
                         + 0.12f * std::sin(kTwoPi * 431.0f * t)
                         + 0.06f * std::sin(kTwoPi * 761.0f * t)
                         + impulse;
        block.setSample(0, sample, tone + deterministicNoise(static_cast<std::uint32_t>(absoluteSample)) * 0.004f);
        block.setSample(1, sample, tone * 0.91f - deterministicNoise(static_cast<std::uint32_t>(absoluteSample + 91)) * 0.004f);
    }
}

void accumulate(const juce::AudioBuffer<float>& block, Metrics& metrics)
{
    for (int ch = 0; ch < block.getNumChannels(); ++ch)
    {
        const float* data = block.getReadPointer(ch);
        for (int i = 0; i < block.getNumSamples(); ++i)
        {
            const float value = data[i];
            metrics.finite = metrics.finite && std::isfinite(value);
            metrics.peak = juce::jmax(metrics.peak, std::abs(value));
            metrics.energy += static_cast<double>(value) * static_cast<double>(value);
        }
    }

    metrics.sampleCount += block.getNumSamples();
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

double computeRms(const Metrics& metrics)
{
    if (metrics.sampleCount <= 0)
        return 0.0;

    return std::sqrt(metrics.energy / static_cast<double>(metrics.sampleCount * kNumChannels));
}

bloomverb::RuntimeParameters makeParams(int type)
{
    bloomverb::RuntimeParameters params;
    params.type = type;
    params.size = 0.64f;
    params.decaySeconds = 5.4f;
    params.preDelayMs = 26.0f;
    params.diffusion = 0.76f;
    params.damping = 0.46f;
    params.early = 0.39f;
    params.width = 1.08f;
    params.mix = 0.78f;
    params.motion = 0.43f;
    params.dynamic = 0.48f;
    params.harmonic = 0.30f;
    params.warp = 0.38f;
    params.swell = 0.22f;
    params.texture = 0.45f;
    params.tone = -0.08f;
    params.lowCutHz = 85.0f;
    params.highCutHz = 11400.0f;
    params.modRateHz = 0.58f;
    params.modDepth = 0.34f;
    params.duckAmount = 0.25f;
    params.bloomAmount = 0.50f;
    params.distance = 0.42f;
    params.transientPreserve = 0.57f;
    return params;
}

bloomverb::RuntimeParameters makeDeterministicParams(int type)
{
    auto params = makeParams(type);
    params.motion = 0.0f;
    params.modDepth = 0.0f;
    params.freeze = false;
    return params;
}

std::string labelFor(int type, double sampleRate, int blockSize)
{
    std::ostringstream label;
    label << "type=" << type << ", sr=" << static_cast<int>(sampleRate) << ", block=" << blockSize;
    return label.str();
}
} // namespace

int main()
{
    bool ok = true;
    std::vector<Result> results;
    results.reserve(8 * kSampleRates.size() * kBlockSizes.size());

    for (int type = 0; type < 8; ++type)
    {
        for (const double sampleRate : kSampleRates)
        {
            const int signalSamples = static_cast<int>(std::round(kSignalSeconds * sampleRate));
            const int tailSamples = static_cast<int>(std::round(kTailSeconds * sampleRate));

            for (const int blockSize : kBlockSizes)
            {
                bloomverb::BloomVerbEngine engine;
                engine.prepare(sampleRate, blockSize, kNumChannels);
                const auto params = makeParams(type);

                Metrics metrics;
                juce::AudioBuffer<float> block(kNumChannels, blockSize);
                std::uint64_t checksum = 1469598103934665603ull;

                int processed = 0;
                while (processed < signalSamples)
                {
                    const int currentBlockSize = juce::jmin(blockSize, signalSamples - processed);
                    if (currentBlockSize != block.getNumSamples())
                        block.setSize(kNumChannels, currentBlockSize, false, false, true);

                    fillInput(block, sampleRate, processed);
                    engine.process(block, params);
                    accumulate(block, metrics);
                    accumulateChecksum(block, checksum);
                    processed += currentBlockSize;
                }

                processed = 0;
                while (processed < tailSamples)
                {
                    const int currentBlockSize = juce::jmin(blockSize, tailSamples - processed);
                    if (currentBlockSize != block.getNumSamples())
                        block.setSize(kNumChannels, currentBlockSize, false, false, true);

                    block.clear();
                    engine.process(block, params);
                    accumulate(block, metrics);
                    accumulateChecksum(block, checksum);
                    processed += currentBlockSize;
                }

                const double rms = computeRms(metrics);
                const auto label = labelFor(type, sampleRate, blockSize);
                ok &= expect(metrics.finite, label + " produced non-finite output");
                ok &= expect(metrics.peak > 1.0e-3f, label + " peak was unexpectedly tiny");
                ok &= expect(metrics.peak < 8.0f, label + " peak exceeded guardrail");
                ok &= expect(rms > 1.0e-4, label + " RMS was unexpectedly tiny");

                results.push_back({ type, sampleRate, blockSize, metrics, rms, checksum });
            }
        }
    }

    for (int type = 0; type < 8; ++type)
    {
        double minRms = std::numeric_limits<double>::max();
        double maxRms = 0.0;

        for (const auto& result : results)
        {
            if (result.type == type)
            {
                minRms = std::min(minRms, result.rms);
                maxRms = std::max(maxRms, result.rms);
            }
        }

        ok &= expect(maxRms / juce::jmax(1.0e-12, minRms) < 3.0, "Per-type RMS spread exceeded bound");
    }

    for (int type = 0; type < 8; ++type)
    {
        for (const double sampleRate : kSampleRates)
        {
            for (const int blockSize : kBlockSizes)
            {
                const auto deterministicParams = makeDeterministicParams(type);

                auto runChecksum = [&](int seedOffset)
                {
                    bloomverb::BloomVerbEngine engine;
                    engine.prepare(sampleRate, blockSize, kNumChannels);
                    juce::AudioBuffer<float> block(kNumChannels, blockSize);
                    std::uint64_t checksum = 1469598103934665603ull + static_cast<std::uint64_t>(seedOffset);

                    const int signalSamples = static_cast<int>(std::round(kSignalSeconds * sampleRate));
                    const int tailSamples = static_cast<int>(std::round(kTailSeconds * sampleRate));

                    int processed = 0;
                    while (processed < signalSamples)
                    {
                        const int currentBlockSize = juce::jmin(blockSize, signalSamples - processed);
                        if (currentBlockSize != block.getNumSamples())
                            block.setSize(kNumChannels, currentBlockSize, false, false, true);

                        fillInput(block, sampleRate, processed);
                        engine.process(block, deterministicParams);
                        accumulateChecksum(block, checksum);
                        processed += currentBlockSize;
                    }

                    processed = 0;
                    while (processed < tailSamples)
                    {
                        const int currentBlockSize = juce::jmin(blockSize, tailSamples - processed);
                        if (currentBlockSize != block.getNumSamples())
                            block.setSize(kNumChannels, currentBlockSize, false, false, true);

                        block.clear();
                        engine.process(block, deterministicParams);
                        accumulateChecksum(block, checksum);
                        processed += currentBlockSize;
                    }

                    return checksum;
                };

                const auto checksumA = runChecksum(0);
                const auto checksumB = runChecksum(0);
                ok &= expect(checksumA == checksumB, labelFor(type, sampleRate, blockSize) + " deterministic replay mismatch");
            }
        }
    }

    if (!ok)
        return 1;

    std::cout << "BloomVerbFDNRegressionTests passed.\n";
    return 0;
}
