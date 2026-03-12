#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace bloomverb::params
{
struct IDs
{
    static constexpr auto type = "type";
    static constexpr auto size = "size";
    static constexpr auto decaySeconds = "decay_s";
    static constexpr auto preDelayMs = "predelay_ms";
    static constexpr auto diffusion = "diffusion";
    static constexpr auto damping = "damping";
    static constexpr auto early = "early";
    static constexpr auto width = "width";
    static constexpr auto mix = "mix";
    static constexpr auto outputDb = "output_db";
    static constexpr auto motion = "motion";
    static constexpr auto dynamic = "dynamic";
    static constexpr auto harmonic = "harmonic";
    static constexpr auto warp = "warp";
    static constexpr auto swell = "swell";
    static constexpr auto texture = "texture";
    static constexpr auto tone = "tone";
    static constexpr auto lowCutHz = "lowcut_hz";
    static constexpr auto highCutHz = "highcut_hz";
    static constexpr auto modRateHz = "mod_rate_hz";
    static constexpr auto modDepth = "mod_depth";
    static constexpr auto duckAmount = "duck_amount";
    static constexpr auto bloomAmount = "bloom_amount";
    static constexpr auto freeze = "freeze";
    static constexpr auto distance = "distance";
    static constexpr auto transientPreserve = "transient_preserve";
};

juce::StringArray getTypeChoices();
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

float getValue(const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef paramID);
bool getBoolValue(const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef paramID);
} // namespace bloomverb::params
