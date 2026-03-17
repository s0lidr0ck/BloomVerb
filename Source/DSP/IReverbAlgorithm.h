#pragma once

namespace bloomverb
{
struct RuntimeParameters;
struct SmoothedRuntimeParameters;
struct TypeDescriptor;
struct FreezeState;

/** Context passed to algorithm processSample. */
struct AlgorithmProcessContext
{
    const SmoothedRuntimeParameters* smoothed = nullptr;
    const TypeDescriptor* typeDescriptor = nullptr;
    const FreezeState* freezeState = nullptr;
    float decayTailControl = 0.63f;
    float decayRoomBias = 0.0f;
    float decayDampingBias = 0.0f;
    float decayWetContour = 1.0f;
    float distanceDirectScale = 1.0f;
    float distanceWetLift = 1.0f;
    float distanceHighCutScale = 1.0f;
    float swellDelayScale = 1.0f;
    int preDelaySamples = 0;
    float motionState = 0.0f;
    float releaseState = 0.5f;
    float lfoPhase = 0.0f;
};

/** Common reverb algorithm interface. Each family implements this. */
class IReverbAlgorithm
{
public:
    virtual ~IReverbAlgorithm() = default;

    virtual void prepare(double sampleRate) = 0;
    virtual void reset() = 0;

    /** Process one sample. Inputs are diffuseL/R (pre-delayed, early-reflected, diffused).
     *  Outputs are wetL/wetR. */
    virtual void processSample(float diffuseL,
                               float diffuseR,
                               const AlgorithmProcessContext& ctx,
                               float& wetL,
                               float& wetR) = 0;
};

/** Algorithm family enumeration. */
enum class ReverbFamily
{
    Plate = 0,
    HallRoom = 1,
    CloudBloomDream = 2,
    GrainSpecial = 3
};

/** Map type index (0..7) to algorithm family. */
ReverbFamily getFamilyForType(int type) noexcept;

/** Map type index to within-family variant (0-based). */
int getVariantForType(int type) noexcept;
} // namespace bloomverb
