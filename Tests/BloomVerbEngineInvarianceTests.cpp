#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr int kNumChannels = 2;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr std::array<double, 3> kSampleRates { 44100.0, 48000.0, 96000.0 };
constexpr std::array<int, 4> kBlockSizes { 32, 64, 257, 1024 };
constexpr double kSignalSeconds = 0.12;
constexpr double kTailSeconds = 0.05;

struct RunMetrics
{
    bool finite = true;
    double inputEnergy = 0.0;
    double outputEnergy = 0.0;
    float peakAbs = 0.0f;
    int outputSamples = 0;
};

struct ConfigResult
{
    double sampleRate = 0.0;
    int blockSize = 0;
    RunMetrics metrics;
    double outputRms = 0.0;
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
    std::uint32_t x = index * 1664525u + 1013904223u;
    x ^= x >> 15;
    x *= 2246822519u;
    const float unit = static_cast<float>(x & 0x00ffffffu) / static_cast<float>(0x00ffffffu);
    return unit * 2.0f - 1.0f;
}

void fillDeterministicInput(juce::AudioBuffer<float>& block, double sampleRate, int absoluteSampleStart)
{
    const int impulsePeriod = juce::jmax(1, static_cast<int>(std::round(0.047 * sampleRate)));
    const int numSamples = block.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const int absoluteSample = absoluteSampleStart + sample;
        const float timeSeconds = static_cast<float>(absoluteSample / sampleRate);
        const float impulse = (absoluteSample % impulsePeriod == 0) ? 1.0f : 0.0f;

        const float tone = 0.16f * std::sin(kTwoPi * 173.0f * timeSeconds)
                         + 0.11f * std::sin(kTwoPi * 619.0f * timeSeconds)
                         + 0.04f * impulse;
        const float nA = deterministicNoise(static_cast<std::uint32_t>(absoluteSample)) * 0.005f;
        const float nB = deterministicNoise(static_cast<std::uint32_t>(absoluteSample + 17)) * 0.005f;

        block.setSample(0, sample, tone + nA);
        block.setSample(1, sample, 0.92f * tone - nB);
    }
}

void accumulateInputMetrics(const juce::AudioBuffer<float>& buffer, RunMetrics& metrics)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const double value = static_cast<double>(data[sample]);
            metrics.inputEnergy += value * value;
        }
    }
}

void accumulateOutputMetrics(const juce::AudioBuffer<float>& buffer, RunMetrics& metrics)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const float value = data[sample];
            if (!std::isfinite(value))
                metrics.finite = false;

            metrics.peakAbs = juce::jmax(metrics.peakAbs, std::abs(value));
            const double v = static_cast<double>(value);
            metrics.outputEnergy += v * v;
        }
    }

    metrics.outputSamples += buffer.getNumSamples();
}

double computeOutputRms(const RunMetrics& metrics)
{
    if (metrics.outputSamples <= 0)
        return 0.0;

    const double denominator = static_cast<double>(metrics.outputSamples * kNumChannels);
    return std::sqrt(metrics.outputEnergy / denominator);
}

bloomverb::RuntimeParameters makeFixedParameters()
{
    bloomverb::RuntimeParameters params;
    params.type = 3;
    params.size = 0.58f;
    params.decaySeconds = 4.2f;
    params.preDelayMs = 18.0f;
    params.diffusion = 0.70f;
    params.damping = 0.43f;
    params.early = 0.41f;
    params.width = 0.95f;
    params.mix = 0.72f;
    params.outputDb = 0.0f;
    params.motion = 0.28f;
    params.dynamic = 0.44f;
    params.harmonic = 0.24f;
    params.warp = 0.31f;
    params.swell = 0.19f;
    params.texture = 0.37f;
    params.tone = -0.1f;
    params.lowCutHz = 90.0f;
    params.highCutHz = 11000.0f;
    params.modRateHz = 0.70f;
    params.modDepth = 0.32f;
    params.duckAmount = 0.27f;
    params.bloomAmount = 0.53f;
    params.freeze = false;
    params.distance = 0.45f;
    params.transientPreserve = 0.62f;
    return params;
}

std::string makeConfigLabel(double sampleRate, int blockSize)
{
    std::ostringstream label;
    label << "sr=" << static_cast<int>(sampleRate) << ", block=" << blockSize;
    return label.str();
}
} // namespace

int main()
{
    const auto params = makeFixedParameters();
    std::vector<ConfigResult> results;
    results.reserve(kSampleRates.size() * kBlockSizes.size());
    bool ok = true;

    for (const double sampleRate : kSampleRates)
    {
        const int signalSamples = static_cast<int>(std::round(kSignalSeconds * sampleRate));
        const int tailSamples = static_cast<int>(std::round(kTailSeconds * sampleRate));

        for (const int blockSize : kBlockSizes)
        {
            bloomverb::BloomVerbEngine engine;
            engine.prepare(sampleRate, blockSize, kNumChannels);

            RunMetrics metrics;
            juce::AudioBuffer<float> block(kNumChannels, blockSize);

            int processed = 0;
            while (processed < signalSamples)
            {
                const int currentBlockSize = juce::jmin(blockSize, signalSamples - processed);
                if (currentBlockSize != block.getNumSamples())
                    block.setSize(kNumChannels, currentBlockSize, false, false, true);

                fillDeterministicInput(block, sampleRate, processed);
                accumulateInputMetrics(block, metrics);
                engine.process(block, params);
                accumulateOutputMetrics(block, metrics);
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
                accumulateOutputMetrics(block, metrics);
                processed += currentBlockSize;
            }

            const double outputRms = computeOutputRms(metrics);
            const double inputRms = std::sqrt(metrics.inputEnergy / static_cast<double>(signalSamples * kNumChannels));
            const double gainRatio = outputRms / juce::jmax(1.0e-12, inputRms);
            const auto label = makeConfigLabel(sampleRate, blockSize);

            ok &= expect(metrics.finite, label + " produced non-finite output");
            ok &= expect(metrics.outputEnergy > 1.0e-6, label + " output energy was unexpectedly tiny");
            ok &= expect(outputRms > 1.0e-4, label + " output RMS was unexpectedly tiny");
            ok &= expect(metrics.peakAbs > 1.0e-3f, label + " output peak was unexpectedly tiny");
            ok &= expect(metrics.peakAbs < 8.0f, label + " output peak exceeded sanity bound");
            ok &= expect(gainRatio > 0.05 && gainRatio < 8.0, label + " output/input gain ratio was out of bounds");

            results.push_back({ sampleRate, blockSize, metrics, outputRms });
        }
    }

    for (const double sampleRate : kSampleRates)
    {
        double minRms = std::numeric_limits<double>::max();
        double maxRms = 0.0;

        for (const auto& result : results)
        {
            if (std::abs(result.sampleRate - sampleRate) < 0.5)
            {
                minRms = std::min(minRms, result.outputRms);
                maxRms = std::max(maxRms, result.outputRms);
            }
        }

        const double spread = maxRms / juce::jmax(1.0e-12, minRms);
        std::ostringstream message;
        message << "Block-size RMS spread too high at sr=" << static_cast<int>(sampleRate)
                << " (ratio=" << spread << ")";
        ok &= expect(spread < 2.0, message.str());
    }

    for (const int blockSize : kBlockSizes)
    {
        double minRms = std::numeric_limits<double>::max();
        double maxRms = 0.0;

        for (const auto& result : results)
        {
            if (result.blockSize == blockSize)
            {
                minRms = std::min(minRms, result.outputRms);
                maxRms = std::max(maxRms, result.outputRms);
            }
        }

        const double spread = maxRms / juce::jmax(1.0e-12, minRms);
        std::ostringstream message;
        message << "Sample-rate RMS spread too high at block=" << blockSize
                << " (ratio=" << spread << ")";
        ok &= expect(spread < 2.8, message.str());
    }

    double globalMinRms = std::numeric_limits<double>::max();
    double globalMaxRms = 0.0;
    for (const auto& result : results)
    {
        globalMinRms = std::min(globalMinRms, result.outputRms);
        globalMaxRms = std::max(globalMaxRms, result.outputRms);
    }

    const double globalSpread = globalMaxRms / juce::jmax(1.0e-12, globalMinRms);
    ok &= expect(globalSpread < 3.0, "Overall RMS spread exceeded invariance bound");

    if (!ok)
        return 1;

    std::cout << "BloomVerbEngine invariance tests passed.\n";
    return 0;
}
