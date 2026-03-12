#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "DSP/BloomVerbEngine.h"
#include "Parameters/BloomVerbParameters.h"
#include "Presets/BloomVerbPreset.h"

#include <atomic>

class BloomVerbAudioProcessor final : public juce::AudioProcessor
{
public:
    BloomVerbAudioProcessor();
    ~BloomVerbAudioProcessor() override = default;

    using AudioProcessor::processBlock;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const noexcept { return apvts; }

    const juce::StringArray& getPresetNames() const noexcept;
    int getCurrentPresetIndex() const noexcept;
    void applyPresetByIndex(int presetIndex);
    void applyNextPreset();
    void applyPreviousPreset();

private:
    bloomverb::RuntimeParameters readRuntimeParameters() const;
    void applyPresetInternal(const bloomverb::presets::BloomVerbPreset& preset);

    juce::AudioProcessorValueTreeState apvts;
    bloomverb::BloomVerbEngine engine;
    juce::StringArray presetNames;
    std::atomic<int> currentPresetIndex { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloomVerbAudioProcessor)
};
