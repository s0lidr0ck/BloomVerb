#pragma once

#include <array>
#include <cstddef>

#include "ReverbFamilyMapping.h"

namespace bloomverb
{
/** Type index (0..7) maps to family + variant:
 *  Plate(0)->Plate/0, Hall(1)->HallRoom/0, Room(2)->HallRoom/1,
 *  Cloud(3)->CloudBloomDream/0, Bloom(4)->CloudBloomDream/1, Dream(5)->CloudBloomDream/2,
 *  Grain(6)->GrainSpecial/0, Chamber(7)->GrainSpecial/1
 */
constexpr size_t kTankLineCount = 8;
using TankLineArray = std::array<float, kTankLineCount>;

struct TypeVoicingProfile
{
    float roomScale = 1.0f;
    float sizeBias = 0.0f;
    float dampingBias = 0.0f;
    float preDelayScale = 1.0f;
    float earlyScale = 1.0f;
    float widthScale = 1.0f;
    float widthGrowth = 1.0f;
    float motionScale = 1.0f;
    float textureBias = 0.0f;

    float earlyTapWeightBias = 0.0f;
    float earlyTapScaleA = 1.0f;
    float earlyTapScaleB = 1.0f;
    float earlyTapScaleC = 1.0f;
    float diffusionBias = 0.0f;
    float harmonicTilt = 0.0f;
    float warpTendency = 0.0f;
    float lowCutScale = 1.0f;
    float highCutScale = 1.0f;
    float wetGainBias = 1.0f;
    float modRateScale = 1.0f;
};

struct TankConstellation
{
    TankLineArray baseDelaySeconds {};
    TankLineArray inputWeightsLeft {};
    TankLineArray inputWeightsRight {};
    TankLineArray outputWeightsLeft {};
    TankLineArray outputWeightsRight {};
};

struct TypeDescriptor
{
    TypeVoicingProfile voicing;
    TankConstellation tank;
};

const TypeDescriptor& getTypeDescriptor(int type) noexcept;
const TypeVoicingProfile& getTypeVoicingProfile(int type) noexcept;
const TankConstellation& getTankConstellation(int type) noexcept;

/** Get descriptor for family + variant. Use when algorithm needs variant-specific voicing. */
const TypeDescriptor& getDescriptorForFamilyVariant(ReverbFamily family, int variant) noexcept;
} // namespace bloomverb
