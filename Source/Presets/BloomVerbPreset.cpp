#include "BloomVerbPreset.h"
#include "BloomVerbPresetBank.h"

namespace bloomverb::presets
{
const std::vector<BloomVerbPreset>& getFactoryPresets()
{
    static const std::vector<BloomVerbPreset> presets = createFactoryPresetBank();
    return presets;
}

juce::StringArray getFactoryPresetNames()
{
    juce::StringArray names;
    names.ensureStorageAllocated(static_cast<int>(getFactoryPresets().size()));
    for (const auto& preset : getFactoryPresets())
        names.add(juce::String(preset.name));
    return names;
}
} // namespace bloomverb::presets
