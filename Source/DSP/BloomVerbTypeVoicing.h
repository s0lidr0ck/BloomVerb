#pragma once

#include <array>
#include <cstddef>

namespace bloomverb
{
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
} // namespace bloomverb
