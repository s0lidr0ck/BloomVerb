#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class BloomVerbAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor&);
    ~BloomVerbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class ParameterModule;
    class ProductionLookAndFeel;
    class LevelMeter;
    class DisplayPanel;

    void setupPresetControls();
    void refreshPresetSelection();
    void setupModules();
    void refreshStandalonePlaybackState();
    void timerCallback() override;

    BloomVerbAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;
    std::unique_ptr<ProductionLookAndFeel> lookAndFeel;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label statusLabel;

    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::TextButton presetPrevButton;
    juce::TextButton presetNextButton;
    bool suppressPresetBoxChange = false;

    juce::ComboBox typeBox;
    juce::Label typeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    juce::Slider outputFader;
    juce::Label outputFaderLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputFaderAttachment;
    std::unique_ptr<LevelMeter> levelMeter;
    std::unique_ptr<DisplayPanel> displayPanel;

#if JucePlugin_Build_Standalone
    juce::TextButton loadFileButton;
    juce::TextButton playFileButton;
    juce::ToggleButton loopFileToggle;
    juce::Label fileStatusLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
#endif

    std::unique_ptr<ParameterModule> spaceModule;
    std::unique_ptr<ParameterModule> bloomModule;
    std::unique_ptr<ParameterModule> characterModule;
    std::unique_ptr<ParameterModule> sculptModule;
    std::unique_ptr<ParameterModule> outputModule;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloomVerbAudioProcessorEditor)
};
