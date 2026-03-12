#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class BloomVerbAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor&);
    ~BloomVerbAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class ParameterPage;

    void setupPageControls();
    void refreshPageVisibility();

    BloomVerbAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    juce::Label titleLabel;
    juce::Label subtitleLabel;

    juce::ComboBox typeBox;
    juce::Label typeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    juce::ComboBox pageBox;
    juce::Label pageLabel;

    std::unique_ptr<ParameterPage> mainPage;
    std::unique_ptr<ParameterPage> characterPage;
    std::unique_ptr<ParameterPage> advancedPage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloomVerbAudioProcessorEditor)
};
