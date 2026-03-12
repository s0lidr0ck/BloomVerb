#include "BloomVerbPresetBank.h"

#include "../Parameters/BloomVerbParameters.h"

#include <initializer_list>
#include <string>
#include <utility>

namespace
{
using Preset = bloomverb::presets::BloomVerbPreset;
using ParameterOverride = std::pair<std::string, float>;

Preset::ParameterValueMap makeDefaultParameterValues()
{
    using IDs = bloomverb::params::IDs;

    return {
        { IDs::type, 1.0f },
        { IDs::size, 0.50f },
        { IDs::decaySeconds, 2.80f },
        { IDs::preDelayMs, 20.0f },
        { IDs::diffusion, 0.65f },
        { IDs::damping, 0.45f },
        { IDs::early, 0.35f },
        { IDs::width, 1.00f },
        { IDs::mix, 0.25f },
        { IDs::outputDb, 0.0f },
        { IDs::motion, 0.30f },
        { IDs::dynamic, 0.35f },
        { IDs::harmonic, 0.20f },
        { IDs::warp, 0.30f },
        { IDs::swell, 0.20f },
        { IDs::texture, 0.25f },
        { IDs::tone, 0.0f },
        { IDs::lowCutHz, 80.0f },
        { IDs::highCutHz, 12000.0f },
        { IDs::modRateHz, 0.60f },
        { IDs::modDepth, 0.30f },
        { IDs::duckAmount, 0.35f },
        { IDs::bloomAmount, 0.50f },
        { IDs::freeze, 0.0f },
        { IDs::distance, 0.40f },
        { IDs::transientPreserve, 0.50f },
    };
}

Preset makePreset(std::string name,
                  std::string category,
                  std::initializer_list<ParameterOverride> overrides)
{
    auto values = makeDefaultParameterValues();
    for (const auto& [parameterId, value] : overrides)
        values[parameterId] = value;

    return { std::move(name), std::move(category), std::move(values) };
}
} // namespace

namespace bloomverb::presets
{
std::vector<BloomVerbPreset> createFactoryPresetBank()
{
    using IDs = bloomverb::params::IDs;
    constexpr auto utility = "Utility";
    constexpr auto ambient = "Ambient";
    constexpr auto guitar = "Guitar";
    constexpr auto vocal = "Vocal";
    constexpr auto cinematic = "Cinematic";

    std::vector<BloomVerbPreset> presets;
    presets.reserve(15);

    presets.push_back(makePreset("Small Plate", utility,
        {
            { IDs::type, 0.0f }, { IDs::size, 0.34f }, { IDs::decaySeconds, 1.20f }, { IDs::preDelayMs, 8.0f },
            { IDs::diffusion, 0.58f }, { IDs::damping, 0.56f }, { IDs::early, 0.52f }, { IDs::width, 0.95f },
            { IDs::mix, 0.18f }, { IDs::motion, 0.10f }, { IDs::dynamic, 0.25f }, { IDs::harmonic, 0.10f },
            { IDs::warp, 0.08f }, { IDs::swell, 0.05f }, { IDs::texture, 0.16f }, { IDs::lowCutHz, 120.0f },
            { IDs::highCutHz, 15000.0f }, { IDs::modDepth, 0.08f }, { IDs::bloomAmount, 0.20f }, { IDs::distance, 0.25f },
            { IDs::transientPreserve, 0.70f },
        }));

    presets.push_back(makePreset("Tight Room", utility,
        {
            { IDs::type, 2.0f }, { IDs::size, 0.22f }, { IDs::decaySeconds, 0.75f }, { IDs::preDelayMs, 2.0f },
            { IDs::diffusion, 0.46f }, { IDs::damping, 0.62f }, { IDs::early, 0.62f }, { IDs::width, 0.90f },
            { IDs::mix, 0.14f }, { IDs::motion, 0.05f }, { IDs::dynamic, 0.18f }, { IDs::harmonic, 0.05f },
            { IDs::warp, 0.04f }, { IDs::swell, 0.02f }, { IDs::texture, 0.10f }, { IDs::lowCutHz, 140.0f },
            { IDs::highCutHz, 13000.0f }, { IDs::duckAmount, 0.25f }, { IDs::bloomAmount, 0.10f }, { IDs::distance, 0.18f },
            { IDs::transientPreserve, 0.85f },
        }));

    presets.push_back(makePreset("Clean Hall", utility,
        {
            { IDs::type, 1.0f }, { IDs::size, 0.48f }, { IDs::decaySeconds, 2.40f }, { IDs::preDelayMs, 18.0f },
            { IDs::diffusion, 0.68f }, { IDs::damping, 0.50f }, { IDs::early, 0.30f }, { IDs::width, 1.10f },
            { IDs::mix, 0.22f }, { IDs::motion, 0.20f }, { IDs::dynamic, 0.30f }, { IDs::harmonic, 0.14f },
            { IDs::warp, 0.16f }, { IDs::texture, 0.24f }, { IDs::lowCutHz, 90.0f }, { IDs::highCutHz, 14500.0f },
            { IDs::duckAmount, 0.30f }, { IDs::bloomAmount, 0.35f }, { IDs::distance, 0.35f }, { IDs::transientPreserve, 0.60f },
        }));

    presets.push_back(makePreset("Bloom Pad", ambient,
        {
            { IDs::type, 4.0f }, { IDs::size, 0.82f }, { IDs::decaySeconds, 10.5f }, { IDs::preDelayMs, 42.0f },
            { IDs::diffusion, 0.83f }, { IDs::damping, 0.36f }, { IDs::early, 0.20f }, { IDs::width, 1.45f },
            { IDs::mix, 0.48f }, { IDs::motion, 0.55f }, { IDs::dynamic, 0.42f }, { IDs::harmonic, 0.32f },
            { IDs::warp, 0.40f }, { IDs::swell, 0.55f }, { IDs::texture, 0.58f }, { IDs::tone, 0.18f },
            { IDs::lowCutHz, 70.0f }, { IDs::highCutHz, 11000.0f }, { IDs::modRateHz, 0.35f }, { IDs::modDepth, 0.56f },
            { IDs::duckAmount, 0.22f }, { IDs::bloomAmount, 0.75f }, { IDs::distance, 0.62f }, { IDs::transientPreserve, 0.40f },
        }));

    presets.push_back(makePreset("Cloud Lift", ambient,
        {
            { IDs::type, 3.0f }, { IDs::size, 0.90f }, { IDs::decaySeconds, 12.8f }, { IDs::preDelayMs, 55.0f },
            { IDs::diffusion, 0.78f }, { IDs::damping, 0.30f }, { IDs::early, 0.14f }, { IDs::width, 1.65f },
            { IDs::mix, 0.55f }, { IDs::motion, 0.68f }, { IDs::dynamic, 0.38f }, { IDs::harmonic, 0.26f },
            { IDs::warp, 0.48f }, { IDs::swell, 0.62f }, { IDs::texture, 0.64f }, { IDs::tone, 0.24f },
            { IDs::lowCutHz, 60.0f }, { IDs::highCutHz, 9800.0f }, { IDs::modRateHz, 0.42f }, { IDs::modDepth, 0.63f },
            { IDs::duckAmount, 0.18f }, { IDs::bloomAmount, 0.82f }, { IDs::distance, 0.72f }, { IDs::transientPreserve, 0.35f },
        }));

    presets.push_back(makePreset("Dream Wash", ambient,
        {
            { IDs::type, 7.0f }, { IDs::size, 0.88f }, { IDs::decaySeconds, 14.5f }, { IDs::preDelayMs, 65.0f },
            { IDs::diffusion, 0.85f }, { IDs::damping, 0.34f }, { IDs::early, 0.12f }, { IDs::width, 1.72f },
            { IDs::mix, 0.58f }, { IDs::motion, 0.74f }, { IDs::dynamic, 0.36f }, { IDs::harmonic, 0.34f },
            { IDs::warp, 0.52f }, { IDs::swell, 0.70f }, { IDs::texture, 0.67f }, { IDs::tone, 0.30f },
            { IDs::lowCutHz, 55.0f }, { IDs::highCutHz, 9000.0f }, { IDs::modRateHz, 0.28f }, { IDs::modDepth, 0.72f },
            { IDs::duckAmount, 0.15f }, { IDs::bloomAmount, 0.88f }, { IDs::distance, 0.78f }, { IDs::transientPreserve, 0.28f },
        }));

    presets.push_back(makePreset("Swell Plate", guitar,
        {
            { IDs::type, 0.0f }, { IDs::size, 0.56f }, { IDs::decaySeconds, 4.20f }, { IDs::preDelayMs, 30.0f },
            { IDs::diffusion, 0.70f }, { IDs::damping, 0.48f }, { IDs::early, 0.36f }, { IDs::width, 1.25f },
            { IDs::mix, 0.33f }, { IDs::motion, 0.34f }, { IDs::dynamic, 0.45f }, { IDs::harmonic, 0.24f },
            { IDs::warp, 0.30f }, { IDs::swell, 0.60f }, { IDs::texture, 0.33f }, { IDs::tone, 0.05f },
            { IDs::lowCutHz, 110.0f }, { IDs::highCutHz, 11500.0f }, { IDs::modRateHz, 0.55f }, { IDs::modDepth, 0.34f },
            { IDs::duckAmount, 0.48f }, { IDs::bloomAmount, 0.52f }, { IDs::distance, 0.46f }, { IDs::transientPreserve, 0.66f },
        }));

    presets.push_back(makePreset("Ambient Bloom", guitar,
        {
            { IDs::type, 4.0f }, { IDs::size, 0.74f }, { IDs::decaySeconds, 7.80f }, { IDs::preDelayMs, 36.0f },
            { IDs::diffusion, 0.76f }, { IDs::damping, 0.40f }, { IDs::early, 0.28f }, { IDs::width, 1.38f },
            { IDs::mix, 0.40f }, { IDs::motion, 0.48f }, { IDs::dynamic, 0.40f }, { IDs::harmonic, 0.30f },
            { IDs::warp, 0.38f }, { IDs::swell, 0.50f }, { IDs::texture, 0.46f }, { IDs::tone, 0.12f },
            { IDs::lowCutHz, 100.0f }, { IDs::highCutHz, 10200.0f }, { IDs::modRateHz, 0.44f }, { IDs::modDepth, 0.45f },
            { IDs::duckAmount, 0.42f }, { IDs::bloomAmount, 0.70f }, { IDs::distance, 0.55f }, { IDs::transientPreserve, 0.58f },
        }));

    presets.push_back(makePreset("Lead Lift", guitar,
        {
            { IDs::type, 1.0f }, { IDs::size, 0.44f }, { IDs::decaySeconds, 2.30f }, { IDs::preDelayMs, 24.0f },
            { IDs::diffusion, 0.62f }, { IDs::damping, 0.52f }, { IDs::early, 0.40f }, { IDs::width, 1.18f },
            { IDs::mix, 0.24f }, { IDs::outputDb, 0.8f }, { IDs::motion, 0.26f }, { IDs::dynamic, 0.50f },
            { IDs::harmonic, 0.22f }, { IDs::warp, 0.22f }, { IDs::swell, 0.28f }, { IDs::texture, 0.24f },
            { IDs::tone, 0.08f }, { IDs::lowCutHz, 140.0f }, { IDs::highCutHz, 12500.0f }, { IDs::modRateHz, 0.66f },
            { IDs::modDepth, 0.25f }, { IDs::duckAmount, 0.55f }, { IDs::bloomAmount, 0.38f }, { IDs::distance, 0.30f },
            { IDs::transientPreserve, 0.78f },
        }));

    presets.push_back(makePreset("Intimate Chamber", vocal,
        {
            { IDs::type, 6.0f }, { IDs::size, 0.36f }, { IDs::decaySeconds, 1.55f }, { IDs::preDelayMs, 14.0f },
            { IDs::diffusion, 0.60f }, { IDs::damping, 0.58f }, { IDs::early, 0.48f }, { IDs::width, 1.05f },
            { IDs::mix, 0.21f }, { IDs::motion, 0.12f }, { IDs::dynamic, 0.42f }, { IDs::harmonic, 0.10f },
            { IDs::warp, 0.12f }, { IDs::swell, 0.10f }, { IDs::texture, 0.18f }, { IDs::tone, 0.10f },
            { IDs::lowCutHz, 150.0f }, { IDs::highCutHz, 14000.0f }, { IDs::modRateHz, 0.40f }, { IDs::modDepth, 0.14f },
            { IDs::duckAmount, 0.52f }, { IDs::bloomAmount, 0.24f }, { IDs::distance, 0.26f }, { IDs::transientPreserve, 0.82f },
        }));

    presets.push_back(makePreset("Modern Hall", vocal,
        {
            { IDs::type, 1.0f }, { IDs::size, 0.58f }, { IDs::decaySeconds, 3.20f }, { IDs::preDelayMs, 26.0f },
            { IDs::diffusion, 0.72f }, { IDs::damping, 0.46f }, { IDs::early, 0.34f }, { IDs::width, 1.20f },
            { IDs::mix, 0.28f }, { IDs::motion, 0.24f }, { IDs::dynamic, 0.46f }, { IDs::harmonic, 0.16f },
            { IDs::warp, 0.20f }, { IDs::swell, 0.22f }, { IDs::texture, 0.30f }, { IDs::tone, 0.14f },
            { IDs::lowCutHz, 130.0f }, { IDs::highCutHz, 15000.0f }, { IDs::modRateHz, 0.52f }, { IDs::modDepth, 0.24f },
            { IDs::duckAmount, 0.58f }, { IDs::bloomAmount, 0.42f }, { IDs::distance, 0.40f }, { IDs::transientPreserve, 0.74f },
        }));

    presets.push_back(makePreset("Air Plate", vocal,
        {
            { IDs::type, 0.0f }, { IDs::size, 0.42f }, { IDs::decaySeconds, 2.10f }, { IDs::preDelayMs, 20.0f },
            { IDs::diffusion, 0.64f }, { IDs::damping, 0.42f }, { IDs::early, 0.38f }, { IDs::width, 1.15f },
            { IDs::mix, 0.26f }, { IDs::motion, 0.18f }, { IDs::dynamic, 0.40f }, { IDs::harmonic, 0.14f },
            { IDs::warp, 0.16f }, { IDs::swell, 0.16f }, { IDs::texture, 0.26f }, { IDs::tone, 0.22f },
            { IDs::lowCutHz, 170.0f }, { IDs::highCutHz, 17000.0f }, { IDs::modRateHz, 0.58f }, { IDs::modDepth, 0.20f },
            { IDs::duckAmount, 0.54f }, { IDs::bloomAmount, 0.32f }, { IDs::distance, 0.34f }, { IDs::transientPreserve, 0.76f },
        }));

    presets.push_back(makePreset("Suspended Space", cinematic,
        {
            { IDs::type, 3.0f }, { IDs::size, 0.96f }, { IDs::decaySeconds, 16.0f }, { IDs::preDelayMs, 82.0f },
            { IDs::diffusion, 0.88f }, { IDs::damping, 0.28f }, { IDs::early, 0.08f }, { IDs::width, 1.80f },
            { IDs::mix, 0.62f }, { IDs::outputDb, -1.0f }, { IDs::motion, 0.70f }, { IDs::dynamic, 0.32f },
            { IDs::harmonic, 0.38f }, { IDs::warp, 0.58f }, { IDs::swell, 0.74f }, { IDs::texture, 0.72f },
            { IDs::tone, 0.18f }, { IDs::lowCutHz, 45.0f }, { IDs::highCutHz, 8200.0f }, { IDs::modRateHz, 0.22f },
            { IDs::modDepth, 0.78f }, { IDs::duckAmount, 0.10f }, { IDs::bloomAmount, 0.92f }, { IDs::distance, 0.86f },
            { IDs::transientPreserve, 0.22f },
        }));

    presets.push_back(makePreset("Distant Signal", cinematic,
        {
            { IDs::type, 5.0f }, { IDs::size, 0.78f }, { IDs::decaySeconds, 11.6f }, { IDs::preDelayMs, 95.0f },
            { IDs::diffusion, 0.82f }, { IDs::damping, 0.41f }, { IDs::early, 0.10f }, { IDs::width, 1.58f },
            { IDs::mix, 0.57f }, { IDs::outputDb, -2.0f }, { IDs::motion, 0.62f }, { IDs::dynamic, 0.28f },
            { IDs::harmonic, 0.42f }, { IDs::warp, 0.64f }, { IDs::swell, 0.66f }, { IDs::texture, 0.80f },
            { IDs::tone, -0.08f }, { IDs::lowCutHz, 220.0f }, { IDs::highCutHz, 7600.0f }, { IDs::modRateHz, 1.20f },
            { IDs::modDepth, 0.48f }, { IDs::duckAmount, 0.12f }, { IDs::bloomAmount, 0.84f }, { IDs::distance, 0.92f },
            { IDs::transientPreserve, 0.18f },
        }));

    presets.push_back(makePreset("Blooming Void", cinematic,
        {
            { IDs::type, 7.0f }, { IDs::size, 1.00f }, { IDs::decaySeconds, 20.0f }, { IDs::preDelayMs, 120.0f },
            { IDs::diffusion, 0.90f }, { IDs::damping, 0.24f }, { IDs::early, 0.05f }, { IDs::width, 1.90f },
            { IDs::mix, 0.68f }, { IDs::outputDb, -3.0f }, { IDs::motion, 0.80f }, { IDs::dynamic, 0.22f },
            { IDs::harmonic, 0.48f }, { IDs::warp, 0.72f }, { IDs::swell, 0.85f }, { IDs::texture, 0.88f },
            { IDs::tone, 0.26f }, { IDs::lowCutHz, 35.0f }, { IDs::highCutHz, 6800.0f }, { IDs::modRateHz, 0.18f },
            { IDs::modDepth, 0.90f }, { IDs::duckAmount, 0.06f }, { IDs::bloomAmount, 1.00f }, { IDs::freeze, 1.0f },
            { IDs::distance, 1.00f }, { IDs::transientPreserve, 0.12f },
        }));

    return presets;
}
} // namespace bloomverb::presets
