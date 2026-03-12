#include "BloomVerbParameters.h"

namespace bloomverb::params
{
namespace
{
std::unique_ptr<juce::AudioParameterFloat> makeFloatParameter(const juce::StringRef parameterID,
                                                              const juce::String& name,
                                                              juce::NormalisableRange<float> range,
                                                              float defaultValue,
                                                              const juce::String& label = {})
{
    auto attributes = juce::AudioParameterFloatAttributes {};
    if (label.isNotEmpty())
        attributes = attributes.withLabel(label);

    return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { parameterID, 1 },
                                                       name,
                                                       range,
                                                       defaultValue,
                                                       attributes);
}
} // namespace

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
    layout.push_back(makeFloatParameter(IDs::size, "Size", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));
    layout.push_back(makeFloatParameter(IDs::decaySeconds, "Decay",
                                        NormalisableRange<float>(0.10f, 20.0f, 0.001f, 0.4f), 2.80f, "s"));
    layout.push_back(makeFloatParameter(IDs::preDelayMs, "PreDelay",
                                        NormalisableRange<float>(0.0f, 250.0f, 0.01f, 0.6f), 20.0f, "ms"));
    layout.push_back(makeFloatParameter(IDs::diffusion, "Diffusion", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    layout.push_back(makeFloatParameter(IDs::damping, "Damping", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    layout.push_back(makeFloatParameter(IDs::early, "Early", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(makeFloatParameter(IDs::width, "Width", NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.00f));
    layout.push_back(makeFloatParameter(IDs::mix, "Mix", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.push_back(makeFloatParameter(IDs::outputDb, "Output",
                                        NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.push_back(makeFloatParameter(IDs::motion, "Motion", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(makeFloatParameter(IDs::dynamic, "Dynamic", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(makeFloatParameter(IDs::harmonic, "Harmonic", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f));
    layout.push_back(makeFloatParameter(IDs::warp, "Warp", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(makeFloatParameter(IDs::swell, "Swell", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f));
    layout.push_back(makeFloatParameter(IDs::texture, "Texture", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.push_back(makeFloatParameter(IDs::tone, "Tone", NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
    layout.push_back(makeFloatParameter(IDs::lowCutHz, "Low Cut",
                                        NormalisableRange<float>(20.0f, 1200.0f, 1.0f, 0.35f), 80.0f, "Hz"));
    layout.push_back(makeFloatParameter(IDs::highCutHz, "High Cut",
                                        NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.35f), 12000.0f, "Hz"));
    layout.push_back(makeFloatParameter(IDs::modRateHz, "Mod Rate",
                                        NormalisableRange<float>(0.01f, 8.0f, 0.001f, 0.45f), 0.60f, "Hz"));
    layout.push_back(makeFloatParameter(IDs::modDepth, "Mod Depth", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.push_back(makeFloatParameter(IDs::duckAmount, "Duck Amount", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    layout.push_back(makeFloatParameter(IDs::bloomAmount, "Bloom Amount", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));
    layout.push_back(std::make_unique<AudioParameterBool>(ParameterID { IDs::freeze, 1 }, "Freeze", false));
    layout.push_back(makeFloatParameter(IDs::distance, "Distance", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.40f));
    layout.push_back(makeFloatParameter(IDs::transientPreserve, "Transient Preserve",
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
