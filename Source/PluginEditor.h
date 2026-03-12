#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class BloomVerbAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor&);
    ~BloomVerbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class ParameterPage;
    class AnalogPlaceholderLookAndFeel;

    void setupPresetControls();
    void refreshPresetSelection();
    void setupPageControls();
    void setupTabButton(juce::TextButton& button, const juce::String& text, int pageIndex);
    void setActivePage(int pageIndex);
    void refreshPageVisibility();

    BloomVerbAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;
    std::unique_ptr<AnalogPlaceholderLookAndFeel> lookAndFeel;

    juce::Label titleLabel;
    juce::Label subtitleLabel;

    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::TextButton presetPrevButton;
    juce::TextButton presetNextButton;
    bool suppressPresetBoxChange = false;

    juce::ComboBox typeBox;
    juce::Label typeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    juce::TextButton mainTabButton;
    juce::TextButton characterTabButton;
    juce::TextButton advancedTabButton;

    std::unique_ptr<ParameterPage> mainPage;
    std::unique_ptr<ParameterPage> characterPage;
    std::unique_ptr<ParameterPage> advancedPage;
    int activePageIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloomVerbAudioProcessorEditor)
};
