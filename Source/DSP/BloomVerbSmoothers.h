#pragma once

#include "BloomVerbEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace bloomverb
{
struct SmoothedRuntimeParameters
{
    int type = 1;
    bool freeze = false;

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
    float distance = 0.40f;
    float transientPreserve = 0.50f;
};

class RuntimeParameterSmoothers
{
public:
    void prepare(double sampleRate)
    {
        sampleRateHz = juce::jmax(1.0, sampleRate);

        resetSmoother(size, 0.035);
        resetSmoother(decaySeconds, 0.085);
        resetSmoother(preDelayMs, 0.030);
        resetSmoother(diffusion, 0.040);
        resetSmoother(damping, 0.040);
        resetSmoother(early, 0.035);
        resetSmoother(width, 0.040);
        resetSmoother(mix, 0.025);
        resetSmoother(outputDb, 0.025);
        resetSmoother(motion, 0.040);
        resetSmoother(dynamic, 0.040);
        resetSmoother(harmonic, 0.040);
        resetSmoother(warp, 0.040);
        resetSmoother(swell, 0.050);
        resetSmoother(texture, 0.045);
        resetSmoother(tone, 0.040);
        resetSmoother(lowCutHz, 0.060);
        resetSmoother(highCutHz, 0.060);
        resetSmoother(modRateHz, 0.050);
        resetSmoother(modDepth, 0.040);
        resetSmoother(duckAmount, 0.040);
        resetSmoother(bloomAmount, 0.050);
        resetSmoother(distance, 0.060);
        resetSmoother(transientPreserve, 0.045);
    }

    void reset(const RuntimeParameters& params)
    {
        current.type = params.type;
        current.freeze = params.freeze;

        setImmediate(size, params.size);
        setImmediate(decaySeconds, params.decaySeconds);
        setImmediate(preDelayMs, params.preDelayMs);
        setImmediate(diffusion, params.diffusion);
        setImmediate(damping, params.damping);
        setImmediate(early, params.early);
        setImmediate(width, params.width);
        setImmediate(mix, params.mix);
        setImmediate(outputDb, params.outputDb);
        setImmediate(motion, params.motion);
        setImmediate(dynamic, params.dynamic);
        setImmediate(harmonic, params.harmonic);
        setImmediate(warp, params.warp);
        setImmediate(swell, params.swell);
        setImmediate(texture, params.texture);
        setImmediate(tone, params.tone);
        setImmediate(lowCutHz, params.lowCutHz);
        setImmediate(highCutHz, params.highCutHz);
        setImmediate(modRateHz, params.modRateHz);
        setImmediate(modDepth, params.modDepth);
        setImmediate(duckAmount, params.duckAmount);
        setImmediate(bloomAmount, params.bloomAmount);
        setImmediate(distance, params.distance);
        setImmediate(transientPreserve, params.transientPreserve);
    }

    void setTargets(const RuntimeParameters& params)
    {
        current.type = params.type;
        current.freeze = params.freeze;

        size.setTargetValue(params.size);
        decaySeconds.setTargetValue(params.decaySeconds);
        preDelayMs.setTargetValue(params.preDelayMs);
        diffusion.setTargetValue(params.diffusion);
        damping.setTargetValue(params.damping);
        early.setTargetValue(params.early);
        width.setTargetValue(params.width);
        mix.setTargetValue(params.mix);
        outputDb.setTargetValue(params.outputDb);
        motion.setTargetValue(params.motion);
        dynamic.setTargetValue(params.dynamic);
        harmonic.setTargetValue(params.harmonic);
        warp.setTargetValue(params.warp);
        swell.setTargetValue(params.swell);
        texture.setTargetValue(params.texture);
        tone.setTargetValue(params.tone);
        lowCutHz.setTargetValue(params.lowCutHz);
        highCutHz.setTargetValue(params.highCutHz);
        modRateHz.setTargetValue(params.modRateHz);
        modDepth.setTargetValue(params.modDepth);
        duckAmount.setTargetValue(params.duckAmount);
        bloomAmount.setTargetValue(params.bloomAmount);
        distance.setTargetValue(params.distance);
        transientPreserve.setTargetValue(params.transientPreserve);
    }

    SmoothedRuntimeParameters getNext()
    {
        current.size = size.getNextValue();
        current.decaySeconds = decaySeconds.getNextValue();
        current.preDelayMs = preDelayMs.getNextValue();
        current.diffusion = diffusion.getNextValue();
        current.damping = damping.getNextValue();
        current.early = early.getNextValue();
        current.width = width.getNextValue();
        current.mix = mix.getNextValue();
        current.outputDb = outputDb.getNextValue();
        current.motion = motion.getNextValue();
        current.dynamic = dynamic.getNextValue();
        current.harmonic = harmonic.getNextValue();
        current.warp = warp.getNextValue();
        current.swell = swell.getNextValue();
        current.texture = texture.getNextValue();
        current.tone = tone.getNextValue();
        current.lowCutHz = lowCutHz.getNextValue();
        current.highCutHz = highCutHz.getNextValue();
        current.modRateHz = modRateHz.getNextValue();
        current.modDepth = modDepth.getNextValue();
        current.duckAmount = duckAmount.getNextValue();
        current.bloomAmount = bloomAmount.getNextValue();
        current.distance = distance.getNextValue();
        current.transientPreserve = transientPreserve.getNextValue();
        return current;
    }

private:
    using Smoother = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>;

    void resetSmoother(Smoother& smoother, double seconds)
    {
        smoother.reset(sampleRateHz, seconds);
        smoother.setCurrentAndTargetValue(0.0f);
    }

    static void setImmediate(Smoother& smoother, float value)
    {
        smoother.setCurrentAndTargetValue(value);
    }

    double sampleRateHz = 44100.0;
    SmoothedRuntimeParameters current;

    Smoother size;
    Smoother decaySeconds;
    Smoother preDelayMs;
    Smoother diffusion;
    Smoother damping;
    Smoother early;
    Smoother width;
    Smoother mix;
    Smoother outputDb;
    Smoother motion;
    Smoother dynamic;
    Smoother harmonic;
    Smoother warp;
    Smoother swell;
    Smoother texture;
    Smoother tone;
    Smoother lowCutHz;
    Smoother highCutHz;
    Smoother modRateHz;
    Smoother modDepth;
    Smoother duckAmount;
    Smoother bloomAmount;
    Smoother distance;
    Smoother transientPreserve;
};
} // namespace bloomverb
