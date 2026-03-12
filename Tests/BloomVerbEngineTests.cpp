#include <cmath>
#include <iostream>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include "DSP/BloomVerbEngine.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr int kNumChannels = 2;
constexpr float kTwoPi = 6.28318530717958647692f;

bool isFiniteBuffer(const juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (!std::isfinite(data[sample]))
                return false;
        }
    }

    return true;
}

double sumSquaredEnergy(const juce::AudioBuffer<float>& buffer)
{
    double total = 0.0;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto v = static_cast<double>(data[sample]);
            total += v * v;
        }
    }

    return total;
}
} // namespace

int main()
{
    bloomverb::BloomVerbEngine engine;
    engine.prepare(kSampleRate, kBlockSize, kNumChannels);

    juce::AudioBuffer<float> block(kNumChannels, kBlockSize);
    juce::Random random(0xB10F1u);

    float phaseL = 0.0f;
    float phaseR = 0.0f;
    const float phaseIncL = kTwoPi * 220.0f / static_cast<float>(kSampleRate);
    const float phaseIncR = kTwoPi * 330.0f / static_cast<float>(kSampleRate);

    std::vector<bloomverb::RuntimeParameters> scenarios;
    scenarios.reserve(2);

    bloomverb::RuntimeParameters base;
    base.mix = 0.60f;
    base.decaySeconds = 6.0f;
    base.motion = 0.50f;
    base.modDepth = 0.65f;
    base.harmonic = 0.35f;
    scenarios.push_back(base);

    auto freezeHeavy = base;
    freezeHeavy.freeze = true;
    freezeHeavy.texture = 0.95f;
    freezeHeavy.warp = 0.85f;
    freezeHeavy.dynamic = 1.0f;
    freezeHeavy.duckAmount = 0.8f;
    scenarios.push_back(freezeHeavy);

    double processedEnergy = 0.0;

    for (int type = 0; type < 8; ++type)
    {
        for (auto parameters : scenarios)
        {
            parameters.type = type;

            for (int blockIndex = 0; blockIndex < 20; ++blockIndex)
            {
                for (int sample = 0; sample < kBlockSize; ++sample)
                {
                    const float noiseL = (random.nextFloat() * 2.0f - 1.0f) * 0.01f;
                    const float noiseR = (random.nextFloat() * 2.0f - 1.0f) * 0.01f;
                    const float inL = 0.15f * std::sin(phaseL) + noiseL;
                    const float inR = 0.15f * std::sin(phaseR) + noiseR;
                    block.setSample(0, sample, inL);
                    block.setSample(1, sample, inR);

                    phaseL += phaseIncL;
                    phaseR += phaseIncR;
                    if (phaseL > kTwoPi)
                        phaseL -= kTwoPi;
                    if (phaseR > kTwoPi)
                        phaseR -= kTwoPi;
                }

                engine.process(block, parameters);

                if (!isFiniteBuffer(block))
                {
                    std::cerr << "BloomVerbEngine produced non-finite output for type=" << type << '\n';
                    return 1;
                }

                processedEnergy += sumSquaredEnergy(block);
            }
        }
    }

    if (processedEnergy < 1e-3)
    {
        std::cerr << "Smoke test failed: processed output energy was unexpectedly tiny.\n";
        return 1;
    }

    engine.reset();
    block.clear();
    bloomverb::RuntimeParameters silenceScenario;
    silenceScenario.freeze = true;
    silenceScenario.mix = 1.0f;

    for (int i = 0; i < 8; ++i)
    {
        engine.process(block, silenceScenario);
        if (!isFiniteBuffer(block))
        {
            std::cerr << "BloomVerbEngine reset/silence pass produced non-finite output.\n";
            return 1;
        }
    }

    std::cout << "BloomVerbEngine smoke test passed.\n";
    return 0;
}
