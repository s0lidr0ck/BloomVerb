#include "PluginEditor.h"

#include "Parameters/BloomVerbParameters.h"

#include <vector>

class BloomVerbAudioProcessorEditor::ParameterPage final : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit ParameterPage(juce::AudioProcessorValueTreeState& state)
        : apvts(state)
    {
    }

    void addKnob(const juce::String& paramID, const juce::String& name)
    {
        auto control = std::make_unique<SliderControl>();
        control->label.setText(name, juce::dontSendNotification);
        control->label.setJustificationType(juce::Justification::centred);

        control->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        control->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        control->slider.setPopupDisplayEnabled(true, false, this);
        control->slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffa58bff));
        control->slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1ff));

        control->attachment = std::make_unique<SliderAttachment>(apvts, paramID, control->slider);

        addAndMakeVisible(control->label);
        addAndMakeVisible(control->slider);
        sliders.push_back(std::move(control));
    }

    void addToggle(const juce::String& paramID, const juce::String& name)
    {
        auto control = std::make_unique<ToggleControl>();
        control->button.setButtonText(name);
        control->button.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffd2c6ff));
        control->button.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        control->attachment = std::make_unique<ButtonAttachment>(apvts, paramID, control->button);

        addAndMakeVisible(control->button);
        toggles.push_back(std::move(control));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        const int columns = 4;
        const int cellWidth = area.getWidth() / columns;
        const int cellHeight = 130;

        for (size_t i = 0; i < sliders.size(); ++i)
        {
            const int row = static_cast<int>(i) / columns;
            const int col = static_cast<int>(i) % columns;
            auto cell = juce::Rectangle<int>(area.getX() + col * cellWidth,
                                             area.getY() + row * cellHeight,
                                             cellWidth,
                                             cellHeight).reduced(8);

            sliders[i]->label.setBounds(cell.removeFromTop(20));
            sliders[i]->slider.setBounds(cell);
        }

        const int usedRows = (static_cast<int>(sliders.size()) + columns - 1) / columns;
        auto toggleArea = juce::Rectangle<int>(area.getX(),
                                               area.getY() + usedRows * cellHeight + 8,
                                               area.getWidth(),
                                               juce::jmax(28, area.getHeight() - usedRows * cellHeight - 8));

        const int toggleWidth = juce::jmax(120, toggleArea.getWidth() / juce::jmax(1, static_cast<int>(toggles.size())));
        for (size_t i = 0; i < toggles.size(); ++i)
        {
            toggles[i]->button.setBounds(toggleArea.getX() + static_cast<int>(i) * toggleWidth,
                                         toggleArea.getY(),
                                         toggleWidth,
                                         24);
        }
    }

private:
    struct SliderControl
    {
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<SliderAttachment> attachment;
    };

    struct ToggleControl
    {
        juce::ToggleButton button;
        std::unique_ptr<ButtonAttachment> attachment;
    };

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<SliderControl>> sliders;
    std::vector<std::unique_ptr<ToggleControl>> toggles;
};

BloomVerbAudioProcessorEditor::BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), apvts(p.getAPVTS())
{
    setSize(980, 660);

    titleLabel.setText("BloomVerb", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(34.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("A living reverb that evolves over time", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc3b4f4));
    addAndMakeVisible(subtitleLabel);

    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    typeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(typeLabel);

    typeBox.addItemList(bloomverb::params::getTypeChoices(), 1);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, bloomverb::params::IDs::type, typeBox);
    addAndMakeVisible(typeBox);

    pageLabel.setText("Page", juce::dontSendNotification);
    pageLabel.setJustificationType(juce::Justification::centredLeft);
    pageLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(pageLabel);

    pageBox.addItem("Main", 1);
    pageBox.addItem("Character", 2);
    pageBox.addItem("Advanced", 3);
    pageBox.setSelectedId(1, juce::dontSendNotification);
    pageBox.onChange = [this] { refreshPageVisibility(); };
    addAndMakeVisible(pageBox);

    setupPageControls();
    refreshPageVisibility();
}

void BloomVerbAudioProcessorEditor::setupPageControls()
{
    mainPage = std::make_unique<ParameterPage>(apvts);
    mainPage->addKnob(bloomverb::params::IDs::size, "Size");
    mainPage->addKnob(bloomverb::params::IDs::decaySeconds, "Decay");
    mainPage->addKnob(bloomverb::params::IDs::preDelayMs, "PreDelay");
    mainPage->addKnob(bloomverb::params::IDs::mix, "Mix");
    mainPage->addKnob(bloomverb::params::IDs::motion, "Motion");
    mainPage->addKnob(bloomverb::params::IDs::texture, "Texture");
    mainPage->addKnob(bloomverb::params::IDs::swell, "Swell");
    addAndMakeVisible(*mainPage);

    characterPage = std::make_unique<ParameterPage>(apvts);
    characterPage->addKnob(bloomverb::params::IDs::dynamic, "Dynamic");
    characterPage->addKnob(bloomverb::params::IDs::harmonic, "Harmonic");
    characterPage->addKnob(bloomverb::params::IDs::warp, "Warp");
    characterPage->addKnob(bloomverb::params::IDs::damping, "Damping");
    characterPage->addKnob(bloomverb::params::IDs::width, "Width");
    characterPage->addKnob(bloomverb::params::IDs::early, "Early");
    addAndMakeVisible(*characterPage);

    advancedPage = std::make_unique<ParameterPage>(apvts);
    advancedPage->addKnob(bloomverb::params::IDs::diffusion, "Diffusion");
    advancedPage->addKnob(bloomverb::params::IDs::tone, "Tone");
    advancedPage->addKnob(bloomverb::params::IDs::lowCutHz, "Low Cut");
    advancedPage->addKnob(bloomverb::params::IDs::highCutHz, "High Cut");
    advancedPage->addKnob(bloomverb::params::IDs::duckAmount, "Duck Amt");
    advancedPage->addKnob(bloomverb::params::IDs::modRateHz, "Mod Rate");
    advancedPage->addKnob(bloomverb::params::IDs::modDepth, "Mod Depth");
    advancedPage->addKnob(bloomverb::params::IDs::bloomAmount, "Bloom Amt");
    advancedPage->addKnob(bloomverb::params::IDs::outputDb, "Output");
    advancedPage->addKnob(bloomverb::params::IDs::distance, "Distance");
    advancedPage->addKnob(bloomverb::params::IDs::transientPreserve, "Transient Pres.");
    advancedPage->addToggle(bloomverb::params::IDs::freeze, "Freeze / Infinite");
    addAndMakeVisible(*advancedPage);
}

void BloomVerbAudioProcessorEditor::refreshPageVisibility()
{
    const auto selected = pageBox.getSelectedId();
    if (mainPage != nullptr)
        mainPage->setVisible(selected == 1);
    if (characterPage != nullptr)
        characterPage->setVisible(selected == 2);
    if (advancedPage != nullptr)
        advancedPage->setVisible(selected == 3);
}

void BloomVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient background(juce::Colour(0xff1a1630), 0.0f, 0.0f,
                                    juce::Colour(0xff080a16), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(background);
    g.fillAll();

    auto panel = getLocalBounds().reduced(10).withTrimmedTop(92);
    g.setColour(juce::Colour(0x221f2848));
    g.fillRoundedRectangle(panel.toFloat(), 10.0f);
    g.setColour(juce::Colour(0x55a58bff));
    g.drawRoundedRectangle(panel.toFloat(), 10.0f, 1.2f);
}

void BloomVerbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(14);
    auto header = area.removeFromTop(84);

    auto titleArea = header.removeFromLeft(520);
    titleLabel.setBounds(titleArea.removeFromTop(44));
    subtitleLabel.setBounds(titleArea.removeFromTop(26));

    auto controls = header.reduced(8, 4);
    auto typeArea = controls.removeFromLeft(210);
    typeLabel.setBounds(typeArea.removeFromTop(18));
    typeBox.setBounds(typeArea.removeFromTop(28));

    auto pageArea = controls.removeFromLeft(210);
    pageLabel.setBounds(pageArea.removeFromTop(18));
    pageBox.setBounds(pageArea.removeFromTop(28));

    const auto pageAreaBounds = area.reduced(6);
    if (mainPage != nullptr)
        mainPage->setBounds(pageAreaBounds);
    if (characterPage != nullptr)
        characterPage->setBounds(pageAreaBounds);
    if (advancedPage != nullptr)
        advancedPage->setBounds(pageAreaBounds);
}
