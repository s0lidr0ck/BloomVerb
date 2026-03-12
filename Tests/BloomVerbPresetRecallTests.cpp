#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "Parameters/BloomVerbParameters.h"
#include "PluginProcessor.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr int kNumChannels = 2;
constexpr float kTolerance = 1.0e-4f;
constexpr float kTwoPi = 6.28318530717958647692f;

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }

    return true;
}

bool isFiniteBuffer(const juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (!std::isfinite(data[sample]))
                return false;
        }
    }

    return true;
}

bool isNearlyEqual(float a, float b, float tolerance = kTolerance)
{
    return std::abs(a - b) <= tolerance;
}

struct Snapshot
{
    float mix = 0.0f;
    float decay = 0.0f;
    float tone = 0.0f;
    float duckAmount = 0.0f;
    float freeze = 0.0f;
    int presetIndex = -1;
    juce::String presetName;
};

Snapshot captureSnapshot(BloomVerbAudioProcessor& processor, const juce::StringArray& names, bool& ok)
{
    using IDs = bloomverb::params::IDs;
    Snapshot snapshot;

    const auto* mix = processor.getAPVTS().getRawParameterValue(IDs::mix);
    const auto* decay = processor.getAPVTS().getRawParameterValue(IDs::decaySeconds);
    const auto* tone = processor.getAPVTS().getRawParameterValue(IDs::tone);
    const auto* duck = processor.getAPVTS().getRawParameterValue(IDs::duckAmount);
    const auto* freeze = processor.getAPVTS().getRawParameterValue(IDs::freeze);

    ok &= expect(mix != nullptr && decay != nullptr && tone != nullptr && duck != nullptr && freeze != nullptr,
                 "Missing key APVTS parameter pointers in snapshot capture");
    if (!(mix != nullptr && decay != nullptr && tone != nullptr && duck != nullptr && freeze != nullptr))
        return snapshot;

    snapshot.mix = mix->load();
    snapshot.decay = decay->load();
    snapshot.tone = tone->load();
    snapshot.duckAmount = duck->load();
    snapshot.freeze = freeze->load();
    snapshot.presetIndex = processor.getCurrentPresetIndex();

    if (snapshot.presetIndex >= 0 && snapshot.presetIndex < names.size())
        snapshot.presetName = names[snapshot.presetIndex];

    return snapshot;
}

bool checkSnapshotFinite(const Snapshot& snapshot, const std::string& prefix)
{
    bool ok = true;
    ok &= expect(std::isfinite(snapshot.mix), prefix + ": mix must be finite");
    ok &= expect(std::isfinite(snapshot.decay), prefix + ": decay must be finite");
    ok &= expect(std::isfinite(snapshot.tone), prefix + ": tone must be finite");
    ok &= expect(std::isfinite(snapshot.duckAmount), prefix + ": duck amount must be finite");
    ok &= expect(std::isfinite(snapshot.freeze), prefix + ": freeze must be finite");
    return ok;
}

void fillSineBlock(juce::AudioBuffer<float>& buffer, float& phaseL, float& phaseR)
{
    const float incL = kTwoPi * 220.0f / static_cast<float>(kSampleRate);
    const float incR = kTwoPi * 330.0f / static_cast<float>(kSampleRate);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        buffer.setSample(0, sample, 0.2f * std::sin(phaseL));
        buffer.setSample(1, sample, 0.2f * std::sin(phaseR));

        phaseL += incL;
        phaseR += incR;
        if (phaseL > kTwoPi)
            phaseL -= kTwoPi;
        if (phaseR > kTwoPi)
            phaseR -= kTwoPi;
    }
}
} // namespace

int main()
{
    bool ok = true;

    BloomVerbAudioProcessor sourceProcessor;
    BloomVerbAudioProcessor restoredProcessor;

    sourceProcessor.prepareToPlay(kSampleRate, kBlockSize);
    restoredProcessor.prepareToPlay(kSampleRate, kBlockSize);

    const auto& presetNames = sourceProcessor.getPresetNames();
    ok &= expect(presetNames.size() >= 2, "Expected at least 2 factory presets");
    if (!ok)
        return 1;

    const int firstPresetIndex = 0;
    const int secondPresetIndex = presetNames.size() > 3 ? 3 : 1;

    sourceProcessor.applyPresetByIndex(firstPresetIndex);
    const auto first = captureSnapshot(sourceProcessor, presetNames, ok);
    ok &= checkSnapshotFinite(first, "First preset snapshot");
    ok &= expect(first.presetIndex == firstPresetIndex, "First preset index did not apply correctly");

    sourceProcessor.applyPresetByIndex(secondPresetIndex);
    const auto second = captureSnapshot(sourceProcessor, presetNames, ok);
    ok &= checkSnapshotFinite(second, "Second preset snapshot");
    ok &= expect(second.presetIndex == secondPresetIndex, "Second preset index did not apply correctly");
    ok &= expect(second.presetName == presetNames[secondPresetIndex], "Second preset name mismatch");

    const bool snapshotsDiffer = !isNearlyEqual(first.mix, second.mix)
        || !isNearlyEqual(first.decay, second.decay)
        || !isNearlyEqual(first.tone, second.tone)
        || !isNearlyEqual(first.duckAmount, second.duckAmount)
        || !isNearlyEqual(first.freeze, second.freeze);
    ok &= expect(snapshotsDiffer, "Selected presets should produce distinct parameter snapshots");

    juce::MemoryBlock state;
    sourceProcessor.getStateInformation(state);
    ok &= expect(state.getSize() > 0, "Serialized state should not be empty");

    restoredProcessor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    const auto restored = captureSnapshot(restoredProcessor, presetNames, ok);
    ok &= checkSnapshotFinite(restored, "Restored snapshot");

    ok &= expect(isNearlyEqual(restored.mix, second.mix), "Mix was not restored correctly");
    ok &= expect(isNearlyEqual(restored.decay, second.decay), "Decay was not restored correctly");
    ok &= expect(isNearlyEqual(restored.tone, second.tone), "Tone was not restored correctly");
    ok &= expect(isNearlyEqual(restored.duckAmount, second.duckAmount), "Duck amount was not restored correctly");
    ok &= expect(isNearlyEqual(restored.freeze, second.freeze), "Freeze was not restored correctly");

    if (restored.presetIndex == second.presetIndex)
    {
        ok &= expect(restored.presetName == second.presetName, "Serialized preset name did not round-trip with index");
    }
    else
    {
        const int expectedDefaultIndex = 0;
        ok &= expect(restored.presetIndex == expectedDefaultIndex,
                     "Preset index expected to reset to default when not serialized");
        if (restored.presetIndex >= 0 && restored.presetIndex < presetNames.size())
            ok &= expect(restored.presetName == presetNames[expectedDefaultIndex],
                         "Default preset name mismatch after restore");
    }

    juce::AudioBuffer<float> audioBlock(kNumChannels, kBlockSize);
    juce::MidiBuffer midi;
    float phaseL = 0.0f;
    float phaseR = 0.0f;

    for (int i = 0; i < 8; ++i)
    {
        fillSineBlock(audioBlock, phaseL, phaseR);
        restoredProcessor.processBlock(audioBlock, midi);
        ok &= expect(isFiniteBuffer(audioBlock), "Restored processor produced non-finite samples");
    }

    if (!ok)
        return 1;

    std::cout << "BloomVerbPresetRecallTests passed.\n";
    return 0;
}
