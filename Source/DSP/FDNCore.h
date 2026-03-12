#pragma once

#include <array>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

namespace bloomverb
{
struct FDNSettings
{
    float size = 0.50f;
    float decaySeconds = 2.80f;
    float damping = 0.45f;
    float diffusion = 0.65f;
    float width = 1.0f;
    float motion = 0.30f;
    float harmonic = 0.20f;
    float warp = 0.30f;
    float texture = 0.25f;
    float dynamic = 0.35f;
    float releaseState = 0.0f;
    float roomScale = 1.0f;
    float freezeBlend = 0.0f;
    float freezeFeedbackBoost = 0.0f;
    float freezeDampingScale = 1.0f;
};

class FDNCore
{
public:
    void prepare(double sampleRate);
    void reset();

    void processSample(float inputLeft,
                       float inputRight,
                       const FDNSettings& settings,
                       float& outputLeft,
                       float& outputRight);

private:
    static constexpr size_t lineCount = 8;

    double sampleRateHz = 44100.0;
    int maxDelaySamples = 1;

    std::array<std::vector<float>, lineCount> delayLines;
    std::array<int, lineCount> writePositions {};
    std::array<float, lineCount> dampStates {};
    std::array<float, lineCount> warpStates {};
    std::array<float, lineCount> modulationPhases {};
};
} // namespace bloomverb
