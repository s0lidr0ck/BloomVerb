#include "GrainSpecialAlgorithm.h"
#include "BloomVerbSmoothers.h"
#include "BloomVerbTypeVoicing.h"
#include "FreezeController.h"

#include <cmath>

namespace bloomverb
{
void GrainSpecialAlgorithm::prepare(double sampleRate)
{
    fdnCore = std::make_unique<FDNCore>();
    fdnCore->prepare(sampleRate);
}

void GrainSpecialAlgorithm::reset()
{
    if (fdnCore)
        fdnCore->reset();
}

void GrainSpecialAlgorithm::processSample(float diffuseL,
                                         float diffuseR,
                                         const AlgorithmProcessContext& ctx,
                                         float& wetL,
                                         float& wetR)
{
    if (!ctx.smoothed || !ctx.typeDescriptor || !ctx.freezeState || !fdnCore)
    {
        wetL = diffuseL;
        wetR = diffuseR;
        return;
    }

    const auto& profile = ctx.typeDescriptor->voicing;
    const auto& freezeState = *ctx.freezeState;
    const auto& smoothed = *ctx.smoothed;

    const float diffusionAmount = juce::jlimit(0.0f, 1.0f,
                                              smoothed.diffusion
                                              + profile.textureBias * smoothed.texture
                                              + profile.diffusionBias);
    const float harmonicTypeScale = juce::jlimit(0.72f, 1.30f, 1.0f + profile.harmonicTilt * 0.26f);
    const float typeSizeScale = juce::jlimit(0.70f, 1.35f, 1.0f + profile.sizeBias * 0.5f);

    FDNSettings fdnSettings;
    fdnSettings.size = juce::jlimit(0.0f, 1.0f, smoothed.size * typeSizeScale + ctx.decayRoomBias);
    fdnSettings.decaySeconds = smoothed.decaySeconds;
    fdnSettings.damping = juce::jlimit(0.0f, 1.0f, smoothed.damping + profile.dampingBias + ctx.decayDampingBias);
    fdnSettings.diffusion = diffusionAmount;
    fdnSettings.width = juce::jlimit(0.0f, 2.5f,
                                     smoothed.width * profile.widthScale
                                         * (1.0f + ctx.releaseState * smoothed.dynamic * 0.20f * profile.widthGrowth));
    fdnSettings.motion = juce::jlimit(0.0f, 1.0f, smoothed.motion * 0.60f + std::abs(ctx.motionState) * 0.10f);
    fdnSettings.harmonic = juce::jlimit(0.0f, 1.0f, smoothed.harmonic * harmonicTypeScale);
    fdnSettings.warp = juce::jlimit(0.0f, 1.0f, smoothed.warp * (1.0f + profile.warpTendency * 0.12f) + 0.22f);
    fdnSettings.texture = juce::jlimit(0.0f, 1.0f, smoothed.texture + 0.30f);
    fdnSettings.dynamic = smoothed.dynamic;
    fdnSettings.releaseState = ctx.releaseState;
    fdnSettings.roomScale = juce::jlimit(0.60f, 1.60f, profile.roomScale + ctx.decayRoomBias);
    fdnSettings.freezeBlend = freezeState.blend;
    fdnSettings.freezeFeedbackBoost = freezeState.feedbackBoost;
    fdnSettings.freezeDampingScale = freezeState.dampingScale;
    fdnSettings.constellation = &ctx.typeDescriptor->tank;

    fdnCore->processSample(diffuseL, diffuseR, fdnSettings, wetL, wetR);
}
} // namespace bloomverb
