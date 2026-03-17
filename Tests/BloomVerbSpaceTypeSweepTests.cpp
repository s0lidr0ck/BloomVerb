#include <cmath>
#include <iostream>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr float kTwoPi = 6.28318530717958647692f;

void fillInput(juce::AudioBuffer<float>& buffer, float& phase)
{
    const float phaseIncA = kTwoPi * 220.0f / static_cast<float>(kSampleRate);
    const float phaseIncB = kTwoPi * 330.0f / static_cast<float>(kSampleRate);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float value = 0.18f * std::sin(phase)
                          + 0.08f * std::sin(phase * (phaseIncB / phaseIncA))
                          + ((sample % 97) == 0 ? 0.06f : 0.0f);
        buffer.setSample(0, sample, value);
        buffer.setSample(1, sample, value);
        phase += phaseIncA;
        if (phase > kTwoPi)
            phase -= kTwoPi;
    }
}

void processBlocks(bloomverb::BloomVerbEngine& engine,
                   bloomverb::RuntimeParameters params,
                   int numBlocks,
                   float& phase)
{
    juce::AudioBuffer<float> block(2, kBlockSize);
    for (int i = 0; i < numBlocks; ++i)
    {
        fillInput(block, phase);
        engine.process(block, params);
    }
}
}

int main()
{
    bloomverb::BloomVerbEngine engine;
    engine.prepare(kSampleRate, kBlockSize, 2);

    float phase = 0.0f;

    for (int type = 0; type < 8; ++type)
    {
        bloomverb::RuntimeParameters params;
        params.type = type;
        params.size = 0.34f;
        params.decaySeconds = 1.20f;
        params.preDelayMs = 8.0f;
        params.distance = 0.25f;
        params.mix = 1.0f;

        processBlocks(engine, params, 24, phase);

        params.size = 0.05f;
        processBlocks(engine, params, 24, phase);
        params.size = 1.0f;
        processBlocks(engine, params, 24, phase);

        params.size = 0.34f;
        params.decaySeconds = 0.15f;
        processBlocks(engine, params, 24, phase);
        params.decaySeconds = 10.0f;
        processBlocks(engine, params, 24, phase);

        params.decaySeconds = 1.20f;
        params.preDelayMs = 0.0f;
        processBlocks(engine, params, 24, phase);
        params.preDelayMs = 120.0f;
        processBlocks(engine, params, 24, phase);

        params.preDelayMs = 8.0f;
        params.distance = 0.0f;
        processBlocks(engine, params, 24, phase);
        params.distance = 1.0f;
        processBlocks(engine, params, 24, phase);

        params.distance = 0.25f;
        params.mix = 0.0f;
        processBlocks(engine, params, 24, phase);
        params.mix = 1.0f;
        processBlocks(engine, params, 24, phase);
    }

    std::cout << "BloomVerbSpaceTypeSweepTests completed.\n";
    return 0;
}
