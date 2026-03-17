#pragma once

#include <array>
#include <memory>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "IReverbAlgorithm.h"

namespace bloomverb
{
class FreezeController;
class RuntimeParameterSmoothers;

struct RuntimeParameters
{
    int type = 1;

    float size = 0.50f;
    float decaySeconds = 2.80f;
    float preDelayMs = 20.0f;
    float diffusion = 0.65f;
    float damping = 0.45f;
    float early = 0.35f;
    float width = 1.00f;
    float mix = 0.25f;
    float outputDb = 0.0f;

    float motion = 0.30f;
    float dynamic = 0.35f;
    float harmonic = 0.20f;
    float warp = 0.30f;
    float swell = 0.20f;
    float texture = 0.25f;

    float tone = 0.0f;
    float lowCutHz = 80.0f;
    float highCutHz = 12000.0f;
    float modRateHz = 0.60f;
    float modDepth = 0.30f;
    float duckAmount = 0.35f;
    float bloomAmount = 0.50f;
    bool freeze = false;
    float distance = 0.40f;
    float transientPreserve = 0.50f;
};

class BloomVerbEngine
{
public:
    BloomVerbEngine();
    ~BloomVerbEngine();

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void process(juce::AudioBuffer<float>& buffer, const RuntimeParameters& parameters);

private:
    static float saturate(float x, float amount);
    static float mapDecaySecondsToTailControl(float decaySeconds);
    void resetTypeDependentState();

    double sampleRateHz = 44100.0;
    int maxSamplesPerBlock = 512;
    int channelCount = 2;
    int delayBufferSize = 0;

    std::array<std::vector<float>, 2> preDelayBuffer;
    std::array<std::vector<float>, 2> earlyDelayBuffer;
    int preDelayWritePos = 0;
    int earlyDelayWritePos = 0;

    std::array<float, 2> diffPrevIn { 0.0f, 0.0f };
    std::array<float, 2> diffPrevOut { 0.0f, 0.0f };
    std::array<float, 2> harmonicRecircState { 0.0f, 0.0f };
    std::array<float, 2> warpTailState { 0.0f, 0.0f };

    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> lowCutFilters;
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> highCutFilters;
    juce::AudioBuffer<float> dryScratchBuffer;
    juce::AudioBuffer<float> wetScratchBuffer;
    std::array<std::unique_ptr<IReverbAlgorithm>, 4> algorithms;
    std::unique_ptr<FreezeController> freezeController;
    std::unique_ptr<RuntimeParameterSmoothers> parameterSmoothers;

    float fastEnvelope = 0.0f;
    float slowEnvelope = 0.0f;
    float releaseState = 0.0f;
    float lfoPhase = 0.0f;
    float motionState = 0.0f;
    float warpContourState = 0.5f;
    float decayTailControl = 0.63f;
    int activeType = -1;
};
} // namespace bloomverb
