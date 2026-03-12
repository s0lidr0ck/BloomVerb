#include "FDNCore.h"

#include <cmath>

namespace
{
constexpr float kTwoPi = 6.28318530717958647692f;

float saturateSample(float x, float amount)
{
    if (amount <= 0.0001f)
        return x;

    const float drive = 1.0f + amount * 5.0f;
    const float normaliser = std::tanh(drive);
    return std::tanh(x * drive) / juce::jmax(0.0001f, normaliser);
}
} // namespace

namespace bloomverb
{
void FDNCore::prepare(double sampleRate)
{
    sampleRateHz = juce::jmax(1.0, sampleRate);
    maxDelaySamples = static_cast<int>(std::ceil(sampleRateHz * 0.40));
    maxDelaySamples = juce::jmax(maxDelaySamples, 64);

    for (auto& line : delayLines)
        line.assign(static_cast<size_t>(maxDelaySamples + 2), 0.0f);

    reset();
}

void FDNCore::reset()
{
    for (auto& line : delayLines)
        std::fill(line.begin(), line.end(), 0.0f);

    writePositions.fill(0);
    dampStates.fill(0.0f);
    warpStates.fill(0.0f);

    for (size_t i = 0; i < lineCount; ++i)
        modulationPhases[i] = static_cast<float>(i) * 0.73f;
}

void FDNCore::processSample(float inputLeft,
                            float inputRight,
                            const FDNSettings& settings,
                            float& outputLeft,
                            float& outputRight)
{
    const auto& constellation = settings.constellation != nullptr
        ? *settings.constellation
        : bloomverb::getTankConstellation(1);
    const float sizeScale = juce::jlimit(0.48f, 1.75f, 0.60f + settings.size * settings.roomScale * 1.05f);
    const float decayNorm = juce::jlimit(0.0f, 1.0f, (settings.decaySeconds - 0.10f) / (20.0f - 0.10f));
    const float feedback = juce::jlimit(0.45f, 0.995f,
                                        0.36f + decayNorm * 0.56f + settings.freezeFeedbackBoost);
    const float dampingCoeff = juce::jlimit(0.01f, 0.24f,
                                            (0.02f + (1.0f - settings.damping) * 0.18f) * settings.freezeDampingScale);
    const float motionDepth = juce::jlimit(0.0f, 1.0f, settings.motion) * 0.010f;
    const float harmonicAmount = juce::jlimit(0.0f, 1.0f, settings.harmonic);
    const float warpAmount = juce::jlimit(0.0f, 1.0f, settings.warp);
    const float textureAmount = juce::jlimit(0.0f, 1.0f, settings.texture);
    const float inputGain = 0.12f + settings.diffusion * 0.08f;

    std::array<float, lineCount> lineValues {};
    float lineAverage = 0.0f;

    for (size_t i = 0; i < lineCount; ++i)
    {
        auto& line = delayLines[i];
        auto& writePos = writePositions[i];

        const float phase = modulationPhases[i];
        const float motionOffset = std::sin(phase) * motionDepth;
        const int delaySamples = juce::jlimit(8, maxDelaySamples,
                                              static_cast<int>(std::round(constellation.baseDelaySeconds[i] * sizeScale * sampleRateHz
                                                                          + motionOffset * sampleRateHz)));
        const int readPos = (writePos - delaySamples + static_cast<int>(line.size())) % static_cast<int>(line.size());

        const float delayed = line[static_cast<size_t>(readPos)];
        dampStates[i] += (delayed - dampStates[i]) * dampingCoeff;
        float lineValue = dampStates[i];

        const float lineDrive = harmonicAmount * (0.12f + 0.05f * static_cast<float>(i % 3));
        lineValue = juce::jmap(lineDrive, lineValue, saturateSample(lineValue, lineDrive * 0.85f));

        const float density = warpAmount * (0.05f + (1.0f - settings.releaseState) * 0.08f);
        warpStates[i] = warpStates[i] * (0.94f + textureAmount * 0.03f) + lineValue * (0.06f - density * 0.02f);
        lineValue = lineValue * (1.0f + density * 0.08f) + warpStates[i] * density * 0.14f;
        lineValue = juce::jlimit(-4.0f, 4.0f, lineValue);

        lineValues[i] = lineValue;
        lineAverage += lineValue;

        modulationPhases[i] += 0.00055f * (0.75f + 0.13f * static_cast<float>(i)) + settings.motion * 0.00035f;
        if (modulationPhases[i] > kTwoPi)
            modulationPhases[i] -= kTwoPi;
    }

    lineAverage /= static_cast<float>(lineCount);

    float tankLeft = 0.0f;
    float tankRight = 0.0f;

    for (size_t i = 0; i < lineCount; ++i)
    {
        auto& line = delayLines[i];
        auto& writePos = writePositions[i];

        const float mixed = -lineValues[i] + 2.0f * lineAverage;
        const float injected = inputGain * (inputLeft * constellation.inputWeightsLeft[i]
                                          + inputRight * constellation.inputWeightsRight[i]);
        const float feedbackSignal = juce::jlimit(-4.0f, 4.0f, mixed * feedback + injected);
        line[static_cast<size_t>(writePos)] = feedbackSignal;
        writePos = (writePos + 1) % static_cast<int>(line.size());

        tankLeft += lineValues[i] * constellation.outputWeightsLeft[i];
        tankRight += lineValues[i] * constellation.outputWeightsRight[i];
    }

    const float width = juce::jlimit(0.0f, 2.0f, settings.width * (0.80f + settings.dynamic * 0.20f));
    const float baseLeft = tankLeft * 0.48f;
    const float baseRight = tankRight * 0.48f;
    const float mid = 0.5f * (baseLeft + baseRight);
    const float side = 0.5f * (baseLeft - baseRight) * width;

    outputLeft = juce::jlimit(-4.0f, 4.0f, mid + side);
    outputRight = juce::jlimit(-4.0f, 4.0f, mid - side);
}
} // namespace bloomverb
