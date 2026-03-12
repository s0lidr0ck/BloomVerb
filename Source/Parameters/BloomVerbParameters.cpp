#include "BloomVerbParameters.h"

namespace bloomverb::params
{
juce::StringArray getTypeChoices()
{
    return {"Plate", "Hall", "Room", "Cloud", "Bloom", "Grain", "Chamber", "Dream"};
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> layout;
    layout.reserve(26);

    layout.push_back(std::make_unique<AudioParameterChoice>(IDs::type, "Type", getTypeChoices(), 1));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::size, 1 }, "Size",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::decaySeconds, 1 }, "Decay",
                                                            NormalisableRange<float>(0.10f, 20.0f, 0.001f, 0.4f), 2.80f, "s"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::preDelayMs, 1 }, "PreDelay",
                                                            NormalisableRange<float>(0.0f, 250.0f, 0.01f, 0.6f), 20.0f, "ms"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::diffusion, 1 }, "Diffusion",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::damping, 1 }, "Damping",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::early, 1 }, "Early",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::width, 1 }, "Width",
                                                            NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.00f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::mix, 1 }, "Mix",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::outputDb, 1 }, "Output",
                                                            NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::motion, 1 }, "Motion",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::dynamic, 1 }, "Dynamic",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::harmonic, 1 }, "Harmonic",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::warp, 1 }, "Warp",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::swell, 1 }, "Swell",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::texture, 1 }, "Texture",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::tone, 1 }, "Tone",
                                                            NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::lowCutHz, 1 }, "Low Cut",
                                                            NormalisableRange<float>(20.0f, 1200.0f, 1.0f, 0.35f), 80.0f, "Hz"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::highCutHz, 1 }, "High Cut",
                                                            NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.35f), 12000.0f, "Hz"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::modRateHz, 1 }, "Mod Rate",
                                                            NormalisableRange<float>(0.01f, 8.0f, 0.001f, 0.45f), 0.60f, "Hz"));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::modDepth, 1 }, "Mod Depth",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::duckAmount, 1 }, "Duck Amount",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::bloomAmount, 1 }, "Bloom Amount",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));
    layout.push_back(std::make_unique<AudioParameterBool>(ParameterID { IDs::freeze, 1 }, "Freeze", false));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::distance, 1 }, "Distance",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.40f));
    layout.push_back(std::make_unique<AudioParameterFloat>(ParameterID { IDs::transientPreserve, 1 }, "Transient Preserve",
                                                            NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));

    return { layout.begin(), layout.end() };
}

float getValue(const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef paramID)
{
    if (const auto* parameter = apvts.getRawParameterValue(paramID))
        return parameter->load();
    return 0.0f;
}

bool getBoolValue(const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef paramID)
{
    return getValue(apvts, paramID) > 0.5f;
}
} // namespace bloomverb::params
