#pragma once

namespace bloomverb
{
struct TypeVoicingProfile
{
    float roomScale = 1.0f;
    float dampingBias = 0.0f;
    float preDelayScale = 1.0f;
    float earlyScale = 1.0f;
    float widthGrowth = 1.0f;
    float motionScale = 1.0f;
    float textureBias = 0.0f;

    float earlyTapWeightBias = 0.0f;
    float diffusionBias = 0.0f;
    float harmonicTilt = 0.0f;
    float warpTendency = 0.0f;
};

TypeVoicingProfile getTypeVoicingProfile(int type) noexcept;
} // namespace bloomverb
