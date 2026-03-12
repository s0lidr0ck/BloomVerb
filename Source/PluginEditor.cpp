#include "PluginEditor.h"

#include "Parameters/BloomVerbParameters.h"

#include <vector>

class BloomVerbAudioProcessorEditor::ProductionLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ProductionLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff4ead8));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff6f5d4b));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff17120d));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1d1712));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff7f6a55));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xfff8efdd));
        setColour(juce::Label::textColourId, juce::Colour(0xfff8efdd));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2f261d));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff8e6d42));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd9c5a6));
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          const float rotaryStartAngle,
                          const float rotaryEndAngle,
                          juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                   static_cast<float>(width), static_cast<float>(height)).reduced(8.0f);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        juce::ColourGradient rimGrad(juce::Colour(0xffb69a76), centre.x, bounds.getY(),
                                     juce::Colour(0xff544332), centre.x, bounds.getBottom(), false);
        g.setGradientFill(rimGrad);
        g.fillEllipse(bounds);

        g.setColour(juce::Colour(0xff110d09));
        g.fillEllipse(bounds.reduced(radius * 0.16f));

        const auto dial = bounds.reduced(radius * 0.28f);
        juce::ColourGradient dialGrad(juce::Colour(0xff7f6b53), dial.getX(), dial.getY(),
                                      juce::Colour(0xff241d16), dial.getRight(), dial.getBottom(), true);
        g.setGradientFill(dialGrad);
        g.fillEllipse(dial);

        juce::Path pointer;
        const float pointerLength = radius * 0.58f;
        const float pointerThickness = 2.8f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);
        g.setColour(juce::Colour(0xfff7e5c8));
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

        g.setColour(juce::Colour(0x8843301a));
        g.drawEllipse(bounds, 1.2f);
    }

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto base = backgroundColour;
        if (isButtonDown)
            base = base.brighter(0.25f);
        else if (isMouseOverButton)
            base = base.brighter(0.12f);

        juce::ColourGradient gradient(base.brighter(0.14f), bounds.getCentreX(), bounds.getY(),
                                      base.darker(0.22f), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0x88dec399));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }
};

class BloomVerbAudioProcessorEditor::LevelMeter final : public juce::Component
{
public:
    void setLevels(float newInputLevel, float newOutputLevel, float freezeAmount)
    {
        inputLevel = juce::jlimit(0.0f, 1.0f, newInputLevel);
        outputLevel = juce::jlimit(0.0f, 1.0f, newOutputLevel);
        freezeBlend = juce::jlimit(0.0f, 1.0f, freezeAmount);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff16110d));
        g.fillRoundedRectangle(area, 8.0f);
        g.setColour(juce::Colour(0xff7f6a55));
        g.drawRoundedRectangle(area, 8.0f, 1.0f);

        auto inner = area.reduced(12.0f, 10.0f);
        auto title = inner.removeFromTop(18.0f);
        g.setColour(juce::Colour(0xffd8c6a8));
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.drawText("Signal", title.toNearestInt(), juce::Justification::centredLeft, false);

        auto bars = inner.removeFromTop(inner.getHeight() - 20.0f);
        const float barGap = 10.0f;
        const float barWidth = (bars.getWidth() - barGap) * 0.5f;
        auto inBar = juce::Rectangle<float>(bars.getX(), bars.getY(), barWidth, bars.getHeight());
        auto outBar = juce::Rectangle<float>(bars.getX() + barWidth + barGap, bars.getY(), barWidth, bars.getHeight());

        drawBar(g, inBar, inputLevel, "IN");
        drawBar(g, outBar, outputLevel, "OUT");

        auto footer = inner;
        const juce::Colour freezeColour = juce::Colour(0xff8fd4ff).interpolatedWith(juce::Colour(0xff2b3a4c), 1.0f - freezeBlend);
        g.setColour(freezeColour);
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::plain)));
        g.drawText(freezeBlend > 0.5f ? "Freeze Engaged" : "Freeze Ready",
                   footer.toNearestInt(), juce::Justification::centredLeft, false);
    }

private:
    static void drawBar(juce::Graphics& g, juce::Rectangle<float> bounds, float level, const juce::String& label)
    {
        g.setColour(juce::Colour(0xff0c0907));
        g.fillRoundedRectangle(bounds, 5.0f);

        auto fill = bounds;
        fill.setY(bounds.getBottom() - bounds.getHeight() * level);
        juce::ColourGradient gradient(juce::Colour(0xff3db78a), fill.getCentreX(), fill.getBottom(),
                                      juce::Colour(0xfff0c36c), fill.getCentreX(), fill.getY(), false);
        if (level > 0.82f)
            gradient.addColour(0.0, juce::Colour(0xffd6524c));
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(fill, 5.0f);

        g.setColour(juce::Colour(0x66f4ead8));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
        g.setColour(juce::Colour(0xffd8c6a8));
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(label, bounds.removeFromBottom(16.0f).toNearestInt(), juce::Justification::centred, false);
    }

    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    float freezeBlend = 0.0f;
};

class BloomVerbAudioProcessorEditor::ParameterPage final : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit ParameterPage(juce::AudioProcessorValueTreeState& state,
                           juce::String name,
                           int rowsIn,
                           int columnsIn)
        : apvts(state), pageName(std::move(name)), rows(rowsIn), columns(columnsIn)
    {
    }

    void addKnob(const juce::String& paramID, const juce::String& name, int row, int column)
    {
        auto control = std::make_unique<SliderControl>();
        control->label.setText(name, juce::dontSendNotification);
        control->label.setJustificationType(juce::Justification::centred);
        control->label.setColour(juce::Label::textColourId, juce::Colour(0xffe5d7c1));

        control->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        control->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        control->slider.setPopupDisplayEnabled(true, false, this);
        control->row = row;
        control->column = column;

        control->attachment = std::make_unique<SliderAttachment>(apvts, paramID, control->slider);

        addAndMakeVisible(control->label);
        addAndMakeVisible(control->slider);
        sliders.push_back(std::move(control));
    }

    void addToggle(const juce::String& paramID, const juce::String& name, int row, int column)
    {
        auto control = std::make_unique<ToggleControl>();
        control->button.setButtonText(name);
        control->button.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xfff2e0c5));
        control->button.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe5d7c1));
        control->row = row;
        control->column = column;
        control->attachment = std::make_unique<ButtonAttachment>(apvts, paramID, control->button);

        addAndMakeVisible(control->button);
        toggles.push_back(std::move(control));
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(8.0f);
        g.setColour(juce::Colour(0xff2a221a));
        g.fillRoundedRectangle(bounds, 9.0f);
        g.setColour(juce::Colour(0xff9b866d));
        g.drawRoundedRectangle(bounds, 9.0f, 1.4f);

        g.setColour(juce::Colour(0xffd6c4aa));
        g.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
        g.drawText(pageName, getLocalBounds().removeFromTop(30).reduced(16, 2), juce::Justification::centredLeft, false);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(18);
        area.removeFromTop(34);

        const int safeRows = juce::jmax(1, rows);
        const int safeColumns = juce::jmax(1, columns);
        const int cellWidth = area.getWidth() / safeColumns;
        const int cellHeight = area.getHeight() / safeRows;

        for (const auto& knob : sliders)
        {
            auto cell = juce::Rectangle<int>(area.getX() + knob->column * cellWidth,
                                             area.getY() + knob->row * cellHeight,
                                             cellWidth,
                                             cellHeight).reduced(8);

            knob->label.setBounds(cell.removeFromTop(22));
            knob->slider.setBounds(cell);
        }

        for (const auto& toggle : toggles)
        {
            auto cell = juce::Rectangle<int>(area.getX() + toggle->column * cellWidth,
                                             area.getY() + toggle->row * cellHeight,
                                             cellWidth,
                                             cellHeight).reduced(10, 20);
            toggle->button.setBounds(cell.withSizeKeepingCentre(cell.getWidth() - 10, 24));
        }
    }

private:
    struct SliderControl
    {
        juce::Label label;
        juce::Slider slider;
        int row = 0;
        int column = 0;
        std::unique_ptr<SliderAttachment> attachment;
    };

    struct ToggleControl
    {
        juce::ToggleButton button;
        int row = 0;
        int column = 0;
        std::unique_ptr<ButtonAttachment> attachment;
    };

    juce::AudioProcessorValueTreeState& apvts;
    juce::String pageName;
    int rows = 1;
    int columns = 1;
    std::vector<std::unique_ptr<SliderControl>> sliders;
    std::vector<std::unique_ptr<ToggleControl>> toggles;
};

BloomVerbAudioProcessorEditor::BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), apvts(p.getAPVTS()),
      lookAndFeel(std::make_unique<ProductionLookAndFeel>()),
      levelMeter(std::make_unique<LevelMeter>())
{
    setLookAndFeel(lookAndFeel.get());
    setSize(1120, 700);
    setResizable(false, false);

    titleLabel.setText("BloomVerb", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(36.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff7ead0));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Character reverb instrument - production local build", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcdb99d));
    addAndMakeVisible(subtitleLabel);

    statusLabel.setText("FDN tail / freeze-safe / local test gate", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8fd4ff));
    statusLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    addAndMakeVisible(statusLabel);

    setupPresetControls();

    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    typeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe5d7c1));
    addAndMakeVisible(typeLabel);

    typeBox.addItemList(bloomverb::params::getTypeChoices(), 1);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, bloomverb::params::IDs::type, typeBox);
    addAndMakeVisible(typeBox);

    setupTabButton(mainTabButton, "MAIN", 0);
    setupTabButton(characterTabButton, "CHARACTER", 1);
    setupTabButton(advancedTabButton, "ADVANCED", 2);

    setupPageControls();
    addAndMakeVisible(*levelMeter);
    setActivePage(0);
    startTimerHz(30);
}

void BloomVerbAudioProcessorEditor::setupPresetControls()
{
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe5d7c1));
    addAndMakeVisible(presetLabel);

    presetPrevButton.setButtonText("<");
    presetPrevButton.onClick = [this]
    {
        processor.applyPreviousPreset();
        refreshPresetSelection();
    };
    addAndMakeVisible(presetPrevButton);

    presetNextButton.setButtonText(">");
    presetNextButton.onClick = [this]
    {
        processor.applyNextPreset();
        refreshPresetSelection();
    };
    addAndMakeVisible(presetNextButton);

    presetBox.addItemList(processor.getPresetNames(), 1);
    presetBox.onChange = [this]
    {
        if (suppressPresetBoxChange)
            return;

        if (const auto presetIndex = presetBox.getSelectedItemIndex(); presetIndex >= 0)
        {
            processor.applyPresetByIndex(presetIndex);
            refreshPresetSelection();
        }
    };
    addAndMakeVisible(presetBox);

    refreshPresetSelection();
}

void BloomVerbAudioProcessorEditor::refreshPresetSelection()
{
    const auto presetCount = processor.getPresetNames().size();
    const bool hasPresets = presetCount > 0;
    const auto presetIndex = processor.getCurrentPresetIndex();

    presetBox.setEnabled(hasPresets);
    presetPrevButton.setEnabled(hasPresets);
    presetNextButton.setEnabled(hasPresets);

    suppressPresetBoxChange = true;
    if (hasPresets && presetIndex >= 0)
        presetBox.setSelectedItemIndex(presetIndex, juce::dontSendNotification);
    else
        presetBox.setSelectedId(0, juce::dontSendNotification);
    suppressPresetBoxChange = false;
}

void BloomVerbAudioProcessorEditor::setupPageControls()
{
    mainPage = std::make_unique<ParameterPage>(apvts, "Main", 2, 4);
    mainPage->addKnob(bloomverb::params::IDs::size, "Size", 0, 0);
    mainPage->addKnob(bloomverb::params::IDs::decaySeconds, "Decay", 0, 1);
    mainPage->addKnob(bloomverb::params::IDs::preDelayMs, "PreDelay", 0, 2);
    mainPage->addKnob(bloomverb::params::IDs::mix, "Mix", 0, 3);
    mainPage->addKnob(bloomverb::params::IDs::motion, "Motion", 1, 0);
    mainPage->addKnob(bloomverb::params::IDs::texture, "Texture", 1, 1);
    mainPage->addKnob(bloomverb::params::IDs::swell, "Swell", 1, 2);
    addAndMakeVisible(*mainPage);

    characterPage = std::make_unique<ParameterPage>(apvts, "Character", 2, 3);
    characterPage->addKnob(bloomverb::params::IDs::dynamic, "Dynamic", 0, 0);
    characterPage->addKnob(bloomverb::params::IDs::harmonic, "Harmonic", 0, 1);
    characterPage->addKnob(bloomverb::params::IDs::warp, "Warp", 0, 2);
    characterPage->addKnob(bloomverb::params::IDs::damping, "Damping", 1, 0);
    characterPage->addKnob(bloomverb::params::IDs::width, "Width", 1, 1);
    characterPage->addKnob(bloomverb::params::IDs::early, "Early", 1, 2);
    addAndMakeVisible(*characterPage);

    advancedPage = std::make_unique<ParameterPage>(apvts, "Advanced", 3, 4);
    advancedPage->addKnob(bloomverb::params::IDs::diffusion, "Diffusion", 0, 0);
    advancedPage->addKnob(bloomverb::params::IDs::tone, "Tone", 0, 1);
    advancedPage->addKnob(bloomverb::params::IDs::lowCutHz, "Low Cut", 0, 2);
    advancedPage->addKnob(bloomverb::params::IDs::highCutHz, "High Cut", 0, 3);
    advancedPage->addKnob(bloomverb::params::IDs::duckAmount, "Duck Amount", 1, 0);
    advancedPage->addKnob(bloomverb::params::IDs::modRateHz, "Mod Rate", 1, 1);
    advancedPage->addKnob(bloomverb::params::IDs::modDepth, "Mod Depth", 1, 2);
    advancedPage->addKnob(bloomverb::params::IDs::bloomAmount, "Bloom Amount", 1, 3);
    advancedPage->addKnob(bloomverb::params::IDs::distance, "Distance", 2, 0);
    advancedPage->addKnob(bloomverb::params::IDs::transientPreserve, "Transient Preserve", 2, 1);
    advancedPage->addKnob(bloomverb::params::IDs::outputDb, "Output", 2, 2);
    advancedPage->addToggle(bloomverb::params::IDs::freeze, "Freeze / Infinite", 2, 3);
    addAndMakeVisible(*advancedPage);
}

void BloomVerbAudioProcessorEditor::setupTabButton(juce::TextButton& button, const juce::String& text, int pageIndex)
{
    button.setButtonText(text);
    button.setClickingTogglesState(true);
    button.onClick = [this, pageIndex] { setActivePage(pageIndex); };
    addAndMakeVisible(button);
}

BloomVerbAudioProcessorEditor::~BloomVerbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void BloomVerbAudioProcessorEditor::setActivePage(int pageIndex)
{
    activePageIndex = juce::jlimit(0, 2, pageIndex);
    mainTabButton.setToggleState(activePageIndex == 0, juce::dontSendNotification);
    characterTabButton.setToggleState(activePageIndex == 1, juce::dontSendNotification);
    advancedTabButton.setToggleState(activePageIndex == 2, juce::dontSendNotification);
    refreshPageVisibility();
}

void BloomVerbAudioProcessorEditor::refreshPageVisibility()
{
    if (mainPage != nullptr)
        mainPage->setVisible(activePageIndex == 0);
    if (characterPage != nullptr)
        characterPage->setVisible(activePageIndex == 1);
    if (advancedPage != nullptr)
        advancedPage->setVisible(activePageIndex == 2);
}

void BloomVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient background(juce::Colour(0xff241b14), 0.0f, 0.0f,
                                    juce::Colour(0xff080605), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(background);
    g.fillAll();

    auto chassis = getLocalBounds().reduced(8).toFloat();
    g.setColour(juce::Colour(0xff120e0b));
    g.fillRoundedRectangle(chassis, 12.0f);
    g.setColour(juce::Colour(0xff8f795f));
    g.drawRoundedRectangle(chassis, 12.0f, 1.4f);

    auto topPlate = chassis.reduced(20.0f, 18.0f).removeFromTop(88.0f);
    juce::ColourGradient plate(juce::Colour(0xff34281d), topPlate.getX(), topPlate.getY(),
                               juce::Colour(0xff17110d), topPlate.getRight(), topPlate.getBottom(), true);
    g.setGradientFill(plate);
    g.fillRoundedRectangle(topPlate, 10.0f);
    g.setColour(juce::Colour(0x55ecd1a8));
    g.drawRoundedRectangle(topPlate, 10.0f, 1.0f);

    g.setColour(juce::Colour(0xff6f5b45));
    constexpr float r = 5.0f;
    g.fillEllipse(chassis.getX() + 10.0f, chassis.getY() + 10.0f, r * 2.0f, r * 2.0f);
    g.fillEllipse(chassis.getRight() - 20.0f, chassis.getY() + 10.0f, r * 2.0f, r * 2.0f);
    g.fillEllipse(chassis.getX() + 10.0f, chassis.getBottom() - 20.0f, r * 2.0f, r * 2.0f);
    g.fillEllipse(chassis.getRight() - 20.0f, chassis.getBottom() - 20.0f, r * 2.0f, r * 2.0f);

    auto footer = chassis.reduced(20.0f, 16.0f).removeFromBottom(26.0f);
    g.setColour(juce::Colour(0xffb49875));
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::plain)));
    g.drawText("Option A console-strip hybrid • fixed control layout • local validation ready",
               footer.toNearestInt(), juce::Justification::centredLeft, false);
}

void BloomVerbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    auto header = area.removeFromTop(132);

    auto titleArea = header.removeFromLeft(760);
    titleLabel.setBounds(titleArea.removeFromTop(44));
    subtitleLabel.setBounds(titleArea.removeFromTop(24));

    auto presetRow = titleArea.removeFromTop(32);
    presetLabel.setBounds(presetRow.removeFromLeft(56));
    presetPrevButton.setBounds(presetRow.removeFromLeft(34));
    presetBox.setBounds(presetRow.removeFromLeft(210).reduced(4, 0));
    presetNextButton.setBounds(presetRow.removeFromLeft(34));

    auto rightRail = header.removeFromRight(170).reduced(4, 2);
    statusLabel.setBounds(rightRail.removeFromTop(24));
    if (levelMeter != nullptr)
        levelMeter->setBounds(rightRail.removeFromTop(92));

    auto controls = header.reduced(8, 8);
    auto typeArea = controls.removeFromTop(50);
    typeLabel.setBounds(typeArea.removeFromLeft(48));
    typeBox.setBounds(typeArea.removeFromLeft(185));

    const int tabHeight = 30;
    const int gap = 8;
    const int tabWidth = (controls.getWidth() - gap * 2) / 3;
    mainTabButton.setBounds(controls.removeFromLeft(tabWidth).withHeight(tabHeight));
    controls.removeFromLeft(gap);
    characterTabButton.setBounds(controls.removeFromLeft(tabWidth).withHeight(tabHeight));
    controls.removeFromLeft(gap);
    advancedTabButton.setBounds(controls.removeFromLeft(tabWidth).withHeight(tabHeight));

    const auto pageAreaBounds = area.reduced(6, 2);
    if (mainPage != nullptr)
        mainPage->setBounds(pageAreaBounds);
    if (characterPage != nullptr)
        characterPage->setBounds(pageAreaBounds);
    if (advancedPage != nullptr)
        advancedPage->setBounds(pageAreaBounds);
}

void BloomVerbAudioProcessorEditor::timerCallback()
{
    const float inputLevel = processor.getInputMeterLevel();
    const float outputLevel = processor.getOutputMeterLevel();
    const float freezeAmount = processor.getFreezeVisualAmount();

    if (levelMeter != nullptr)
        levelMeter->setLevels(inputLevel, outputLevel, freezeAmount);

    const auto presetCount = processor.getPresetNames().size();
    const auto presetIndex = processor.getCurrentPresetIndex();
    juce::String statusText;

    if (freezeAmount > 0.5f)
        statusText << "Freeze engaged";
    else if (outputLevel > 0.85f)
        statusText << "Output hot";
    else if (inputLevel < 0.01f && outputLevel < 0.01f)
        statusText << "Input silent";
    else
        statusText << "Freeze ready";

    statusText << "  •  ";
    if (presetCount > 0 && presetIndex >= 0)
        statusText << "Preset " << (presetIndex + 1) << "/" << presetCount;
    else
        statusText << "No presets";

    if (statusLabel.getText() != statusText)
        statusLabel.setText(statusText, juce::dontSendNotification);
}
