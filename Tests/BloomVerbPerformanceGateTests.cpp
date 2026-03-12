#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr int kNumChannels = 2;
constexpr int kWarmupBlocks = 50;
constexpr int kMeasuredBlocks = 400;
constexpr int kInstanceCount = 4;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr double kRealtimeFractionBudget = 0.75;
constexpr double kMultiInstanceRealtimeFractionBudget = 2.35;

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }

    return true;
}

void fillInput(juce::AudioBuffer<float>& block, float& phaseL, float& phaseR)
{
    const float incL = kTwoPi * 220.0f / static_cast<float>(kSampleRate);
    const float incR = kTwoPi * 329.0f / static_cast<float>(kSampleRate);

    for (int sample = 0; sample < block.getNumSamples(); ++sample)
    {
        block.setSample(0, sample, 0.18f * std::sin(phaseL));
        block.setSample(1, sample, 0.18f * std::sin(phaseR));
        phaseL += incL;
        phaseR += incR;
        if (phaseL > kTwoPi)
            phaseL -= kTwoPi;
        if (phaseR > kTwoPi)
            phaseR -= kTwoPi;
    }
}
} // namespace

int main()
{
    bloomverb::RuntimeParameters params;
    params.type = 3;
    params.size = 0.67f;
    params.decaySeconds = 6.1f;
    params.preDelayMs = 24.0f;
    params.diffusion = 0.81f;
    params.damping = 0.42f;
    params.early = 0.38f;
    params.width = 1.12f;
    params.mix = 0.75f;
    params.motion = 0.50f;
    params.dynamic = 0.52f;
    params.harmonic = 0.31f;
    params.warp = 0.40f;
    params.swell = 0.27f;
    params.texture = 0.58f;
    params.modRateHz = 0.84f;
    params.modDepth = 0.41f;
    params.duckAmount = 0.22f;
    params.bloomAmount = 0.57f;
    params.distance = 0.46f;
    params.transientPreserve = 0.59f;

    std::vector<bloomverb::BloomVerbEngine> engines(static_cast<size_t>(kInstanceCount));
    std::vector<juce::AudioBuffer<float>> blocks;
    blocks.reserve(static_cast<size_t>(kInstanceCount));
    std::vector<float> phasesL(static_cast<size_t>(kInstanceCount), 0.0f);
    std::vector<float> phasesR(static_cast<size_t>(kInstanceCount), 0.0f);

    for (auto& engine : engines)
        engine.prepare(kSampleRate, kBlockSize, kNumChannels);
    for (int i = 0; i < kInstanceCount; ++i)
        blocks.emplace_back(kNumChannels, kBlockSize);

    bool finite = true;

    for (int i = 0; i < kWarmupBlocks; ++i)
    {
        for (int instance = 0; instance < kInstanceCount; ++instance)
        {
            auto localParams = params;
            localParams.type = (params.type + instance) % 8;
            localParams.freeze = (instance == kInstanceCount - 1) && ((i / 3) % 2 == 0);
            localParams.warp = juce::jlimit(0.0f, 1.0f, params.warp + 0.06f * static_cast<float>(instance));
            fillInput(blocks[static_cast<size_t>(instance)], phasesL[static_cast<size_t>(instance)], phasesR[static_cast<size_t>(instance)]);
            engines[static_cast<size_t>(instance)].process(blocks[static_cast<size_t>(instance)], localParams);
        }
    }

    const auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kMeasuredBlocks; ++i)
    {
        for (int instance = 0; instance < kInstanceCount; ++instance)
        {
            auto localParams = params;
            localParams.type = (params.type + instance) % 8;
            localParams.freeze = (instance == kInstanceCount - 1) && ((i / 2) % 2 == 0);
            localParams.texture = juce::jlimit(0.0f, 1.0f, params.texture + 0.07f * static_cast<float>(instance));
            localParams.warp = juce::jlimit(0.0f, 1.0f, params.warp + 0.06f * static_cast<float>(instance));
            localParams.motion = juce::jlimit(0.0f, 1.0f, params.motion + 0.05f * static_cast<float>(instance));

            auto& block = blocks[static_cast<size_t>(instance)];
            fillInput(block, phasesL[static_cast<size_t>(instance)], phasesR[static_cast<size_t>(instance)]);
            engines[static_cast<size_t>(instance)].process(block, localParams);

            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                const float* data = block.getReadPointer(ch);
                for (int sample = 0; sample < block.getNumSamples(); ++sample)
                    finite = finite && std::isfinite(data[sample]);
            }
        }
    }
    const auto stop = std::chrono::high_resolution_clock::now();

    const double elapsedMs = std::chrono::duration<double, std::milli>(stop - start).count();
    const double averageBlockMs = elapsedMs / static_cast<double>(kMeasuredBlocks);
    const double averagePerInstanceBlockMs = averageBlockMs / static_cast<double>(kInstanceCount);
    const double realtimeBudgetMs = static_cast<double>(kBlockSize) * 1000.0 / kSampleRate;

    bool ok = true;
    ok &= expect(finite, "Performance gate encountered non-finite output");
    ok &= expect(averagePerInstanceBlockMs < realtimeBudgetMs * kRealtimeFractionBudget,
                 "Per-instance average block processing time exceeded budget");
    ok &= expect(averageBlockMs < realtimeBudgetMs * kMultiInstanceRealtimeFractionBudget,
                 "Multi-instance aggregate block processing time exceeded budget");

    if (!ok)
        return 1;

    std::cout << "BloomVerbPerformanceGateTests passed. avgBlockMs=" << averageBlockMs
              << " perInstanceBudgetMs=" << realtimeBudgetMs * kRealtimeFractionBudget
              << " multiInstanceBudgetMs=" << realtimeBudgetMs * kMultiInstanceRealtimeFractionBudget << '\n';
    return 0;
}
