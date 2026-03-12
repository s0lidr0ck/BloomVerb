#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace bloomverb::presets
{
struct BloomVerbPreset
{
    using ParameterValueMap = std::unordered_map<std::string, float>;

    std::string name;
    std::string category;
    ParameterValueMap parameterValuesById;
};

const std::vector<BloomVerbPreset>& getFactoryPresets();
juce::StringArray getFactoryPresetNames();
} // namespace bloomverb::presets
