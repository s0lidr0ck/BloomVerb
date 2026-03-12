#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace bloomverb
{
struct FreezeState
{
    float blend = 0.0f;
    float inputGain = 1.0f;
    float feedbackBoost = 0.0f;
    float dampingScale = 1.0f;
};

class FreezeController
{
public:
    void prepare(double sampleRate);
    void reset();
    void setTarget(bool shouldFreeze);
    FreezeState getNextState();

private:
    double sampleRateHz = 44100.0;
    bool targetFrozen = false;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> freezeBlend;
};
} // namespace bloomverb
