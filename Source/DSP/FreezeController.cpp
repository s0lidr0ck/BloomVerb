#include "FreezeController.h"

namespace bloomverb
{
void FreezeController::prepare(double sampleRate)
{
    sampleRateHz = juce::jmax(1.0, sampleRate);
    freezeBlend.reset(sampleRateHz, 0.060);
    freezeBlend.setCurrentAndTargetValue(0.0f);
    targetFrozen = false;
}

void FreezeController::reset()
{
    freezeBlend.setCurrentAndTargetValue(0.0f);
    targetFrozen = false;
}

void FreezeController::setTarget(bool shouldFreeze)
{
    targetFrozen = shouldFreeze;
    freezeBlend.setTargetValue(targetFrozen ? 1.0f : 0.0f);
}

FreezeState FreezeController::getNextState()
{
    const float blend = freezeBlend.getNextValue();

    FreezeState state;
    state.blend = blend;
    state.inputGain = juce::jlimit(0.05f, 1.0f, 1.0f - blend * 0.95f);
    state.feedbackBoost = blend * 0.10f;
    state.dampingScale = juce::jlimit(0.25f, 1.0f, 1.0f - blend * 0.68f);
    return state;
}
} // namespace bloomverb
