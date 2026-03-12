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

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
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
    float getInputMeterLevel() const noexcept { return inputMeterLevel.load(); }
    float getOutputMeterLevel() const noexcept { return outputMeterLevel.load(); }
    float getFreezeVisualAmount() const noexcept { return freezeVisualAmount.load(); }

private:
    bloomverb::RuntimeParameters readRuntimeParameters() const;
    void applyPresetInternal(const bloomverb::presets::BloomVerbPreset& preset);
    void updateMeterValue(std::atomic<float>& meter, float target) const;

    juce::AudioProcessorValueTreeState apvts;
    bloomverb::BloomVerbEngine engine;
    juce::StringArray presetNames;
    std::atomic<int> currentPresetIndex { 0 };
    std::atomic<float> inputMeterLevel { 0.0f };
    std::atomic<float> outputMeterLevel { 0.0f };
    std::atomic<float> freezeVisualAmount { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloomVerbAudioProcessor)
};
