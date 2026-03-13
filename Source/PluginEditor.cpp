#include "PluginEditor.h"

#include "Parameters/BloomVerbParameters.h"

#include <array>
#include <vector>

namespace
{
const auto chassisOuter = juce::Colour(0xff0f1012);
const auto chassisInner = juce::Colour(0xff191713);
const auto brass = juce::Colour(0xffb79b72);
const auto warmGrey = juce::Colour(0xffd8c9b4);
const auto cyanAccent = juce::Colour(0xff77c6d8);
const auto mintAccent = juce::Colour(0xff82c4a5);
const auto redAccent = juce::Colour(0xffc06d63);
const auto screenAccent = juce::Colour(0xff37a7ca);
const auto stripCool = juce::Colour(0xffa8b6bc);
const auto stripWarm = juce::Colour(0xffc0b39d);
const auto stripNeutral = juce::Colour(0xff9ca3ac);

void drawScrews(juce::Graphics& g, juce::Rectangle<float> area)
{
    const float radius = 5.0f;
    const auto drawScrew = [&g, radius](juce::Point<float> centre)
    {
        const auto screwArea = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(centre);
        juce::ColourGradient grad(juce::Colour(0xff8a7a62), screwArea.getCentreX(), screwArea.getY(),
                                  juce::Colour(0xff2a241d), screwArea.getCentreX(), screwArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillEllipse(screwArea);
        g.setColour(juce::Colour(0xaa110f0d));
        g.drawEllipse(screwArea, 0.9f);
        g.drawLine(screwArea.getX() + 3.0f, screwArea.getCentreY(), screwArea.getRight() - 3.0f, screwArea.getCentreY(), 0.9f);
    };

    drawScrew({ area.getX() + 12.0f, area.getY() + 12.0f });
    drawScrew({ area.getRight() - 12.0f, area.getY() + 12.0f });
    drawScrew({ area.getX() + 12.0f, area.getBottom() - 12.0f });
    drawScrew({ area.getRight() - 12.0f, area.getBottom() - 12.0f });
}

void drawPlate(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base, float corner = 10.0f)
{
    juce::ColourGradient fill(base.brighter(0.12f), bounds.getCentreX(), bounds.getY(),
                              base.darker(0.26f), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(fill);
    g.fillRoundedRectangle(bounds, corner);
    g.setColour(base.brighter(0.25f));
    g.drawRoundedRectangle(bounds, corner, 1.0f);
    g.setColour(base.darker(0.55f).withAlpha(0.85f));
    g.drawRoundedRectangle(bounds.reduced(1.6f), juce::jmax(2.0f, corner - 2.0f), 0.8f);
}
}

class BloomVerbAudioProcessorEditor::ProductionLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ProductionLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff6eee0));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff6a6258));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a1c1f));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff191b1f));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff5d5246));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffefe5d6));
        setColour(juce::Label::textColourId, juce::Colour(0xffefe5d6));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff202126));
        setColour(juce::TextButton::buttonOnColourId, brass);
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffdfd1bd));
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xfffef9ef));
        setColour(juce::ToggleButton::textColourId, juce::Colour(0xffebe1d1));
        setColour(juce::CaretComponent::caretColourId, juce::Colour(0xfffef3dd));
    }

    void drawRotarySlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          const float rotaryStartAngle,
                          const float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height)).reduced(10.0f);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId)
                                .interpolatedWith(brass, 0.35f);

        juce::Path tickArc;
        tickArc.addCentredArc(centre.x, centre.y, radius * 0.96f, radius * 0.96f, 0.0f,
                              rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0x556d6559));
        g.strokePath(tickArc, juce::PathStrokeType(2.0f));

        for (int i = 0; i < 11; ++i)
        {
            const float amount = static_cast<float>(i) / 10.0f;
            const float tickAngle = rotaryStartAngle + amount * (rotaryEndAngle - rotaryStartAngle);
            const auto inner = juce::Point<float>(centre.x + std::cos(tickAngle) * radius * 0.82f,
                                                  centre.y + std::sin(tickAngle) * radius * 0.82f);
            const auto outer = juce::Point<float>(centre.x + std::cos(tickAngle) * radius * 0.96f,
                                                  centre.y + std::sin(tickAngle) * radius * 0.96f);
            g.setColour(amount <= sliderPos ? accent.brighter(0.15f) : juce::Colour(0xff4c463f));
            g.drawLine(inner.x, inner.y, outer.x, outer.y, i % 5 == 0 ? 1.8f : 1.0f);
        }

        juce::ColourGradient outerRing(juce::Colour(0xffa8a09a), centre.x, bounds.getY(),
                                       juce::Colour(0xff2a2927), centre.x, bounds.getBottom(), false);
        g.setGradientFill(outerRing);
        g.fillEllipse(bounds);

        const auto bezel = bounds.reduced(radius * 0.11f);
        juce::ColourGradient bezelGrad(juce::Colour(0xff09090a), bezel.getCentreX(), bezel.getY(),
                                       juce::Colour(0xff313239), bezel.getCentreX(), bezel.getBottom(), false);
        g.setGradientFill(bezelGrad);
        g.fillEllipse(bezel);

        const auto dial = bezel.reduced(radius * 0.13f);
        juce::ColourGradient dialGrad(juce::Colour(0xff68615d), dial.getX(), dial.getY(),
                                      juce::Colour(0xff22242a), dial.getRight(), dial.getBottom(), true);
        g.setGradientFill(dialGrad);
        g.fillEllipse(dial);

        const auto cap = dial.reduced(radius * 0.22f);
        g.setColour(accent.withAlpha(0.82f));
        g.fillEllipse(cap);
        g.setColour(accent.brighter(0.18f));
        g.drawEllipse(cap, 1.0f);

        juce::Path pointer;
        const float pointerLength = radius * 0.56f;
        const float pointerThickness = 4.0f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.6f);
        g.setColour(juce::Colour(0xfff8f1e6));
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

        g.setColour(juce::Colour(0x995e5347));
        g.drawEllipse(bounds, 1.0f);
    }

    void drawLinearSlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float,
                          float,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearBarVertical)
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
            return;
        }

        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height)).reduced(4.0f, 2.0f);
        drawPlate(g, bounds, juce::Colour(0xff17191c), 7.0f);

        auto track = bounds.reduced(bounds.getWidth() * 0.33f, 12.0f);
        track.setX(bounds.getCentreX() - track.getWidth() * 0.5f);
        drawPlate(g, track, juce::Colour(0xff0f1215), 4.0f);

        for (int i = 0; i < 9; ++i)
        {
            const float amount = static_cast<float>(i) / 8.0f;
            const float markY = juce::jmap(amount, 0.0f, 1.0f, track.getBottom(), track.getY());
            g.setColour(juce::Colour(0x557f8d97));
            g.drawLine(track.getRight() + 4.0f, markY, bounds.getRight() - 3.0f, markY, i % 2 == 0 ? 1.1f : 0.8f);
        }

        const float clampedPos = juce::jlimit(track.getY(), track.getBottom(), sliderPos);
        auto fill = track;
        fill.setY(clampedPos);
        juce::ColourGradient fillGrad(juce::Colour(0xff2ca6c3), fill.getCentreX(), fill.getBottom(),
                                      juce::Colour(0xffd0ad76), fill.getCentreX(), fill.getY(), false);
        g.setGradientFill(fillGrad);
        g.fillRoundedRectangle(fill, 3.0f);

        auto thumb = juce::Rectangle<float>(bounds.getWidth() - 14.0f, 16.0f)
                         .withCentre({ bounds.getCentreX(), clampedPos });
        juce::ColourGradient thumbFill(juce::Colour(0xffddd4c8), thumb.getCentreX(), thumb.getY(),
                                       juce::Colour(0xff81786d), thumb.getCentreX(), thumb.getBottom(), false);
        g.setGradientFill(thumbFill);
        g.fillRoundedRectangle(thumb, 3.0f);
        g.setColour(juce::Colour(0xbb2b2520));
        g.drawRoundedRectangle(thumb, 3.0f, 0.9f);
    }

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto base = button.getToggleState() ? brass.darker(0.18f) : backgroundColour;
        if (isButtonDown)
            base = base.brighter(0.18f);
        else if (isMouseOverButton)
            base = base.brighter(0.10f);

        drawPlate(g, bounds, base, 7.0f);
    }

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool isMouseOverButton,
                          bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto switchArea = bounds.removeFromLeft(56.0f).reduced(4.0f, 8.0f);
        auto bodyColour = button.getToggleState() ? brass.darker(0.15f) : juce::Colour(0xff202228);
        if (isButtonDown)
            bodyColour = bodyColour.brighter(0.14f);
        else if (isMouseOverButton)
            bodyColour = bodyColour.brighter(0.08f);

        drawPlate(g, switchArea, bodyColour, 6.0f);

        auto latch = switchArea.reduced(6.0f, 6.0f);
        latch.setWidth(latch.getWidth() * 0.48f);
        if (button.getToggleState())
            latch.setX(switchArea.getRight() - latch.getWidth() - 6.0f);

        juce::ColourGradient latchGrad(juce::Colour(0xffd2c6b4), latch.getCentreX(), latch.getY(),
                                       juce::Colour(0xff5f5953), latch.getCentreX(), latch.getBottom(), false);
        g.setGradientFill(latchGrad);
        g.fillRoundedRectangle(latch, 4.0f);
        g.setColour(juce::Colour(0xaa201d1a));
        g.drawRoundedRectangle(latch, 4.0f, 0.9f);

        auto textArea = bounds.toNearestInt().withTrimmedLeft(62);
        g.setColour(findColour(juce::ToggleButton::textColourId));
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawFittedText(button.getButtonText(), textArea, juce::Justification::centredLeft, 1);
    }

    void drawComboBox(juce::Graphics& g,
                      int width,
                      int height,
                      bool,
                      int buttonX,
                      int buttonY,
                      int buttonW,
                      int buttonH,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);
        drawPlate(g, bounds, juce::Colour(0xff1e2024), 7.0f);

        auto arrowArea = juce::Rectangle<float>(static_cast<float>(buttonX), static_cast<float>(buttonY),
                                                static_cast<float>(buttonW), static_cast<float>(buttonH)).reduced(6.0f, 9.0f);
        juce::Path arrow;
        arrow.startNewSubPath(arrowArea.getX(), arrowArea.getY());
        arrow.lineTo(arrowArea.getCentreX(), arrowArea.getBottom());
        arrow.lineTo(arrowArea.getRight(), arrowArea.getY());
        g.setColour(box.isEnabled() ? juce::Colour(0xffeedcc3) : juce::Colour(0xff7b736a));
        g.strokePath(arrow, juce::PathStrokeType(1.8f));
    }
};

class BloomVerbAudioProcessorEditor::LevelMeter final : public juce::Component
{
public:
    void setLevels(float newInputLevel, float newOutputLevel, float newFreezeAmount)
    {
        inputLevel = juce::jlimit(0.0f, 1.0f, newInputLevel);
        outputLevel = juce::jlimit(0.0f, 1.0f, newOutputLevel);
        freezeAmount = juce::jlimit(0.0f, 1.0f, newFreezeAmount);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(4.0f);
        drawPlate(g, area, juce::Colour(0xff1a1816), 14.0f);
        drawScrews(g, area);

        auto inner = area.reduced(16.0f, 16.0f);
        auto header = inner.removeFromTop(38.0f);
        auto badge = header.removeFromLeft(94.0f);
        drawPlate(g, badge, juce::Colour(0xff2b2722), 5.0f);
        g.setColour(warmGrey);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText("Signal", badge.toNearestInt(), juce::Justification::centred, false);

        auto lampArea = header.removeFromRight(56.0f).reduced(10.0f, 6.0f);
        const auto lampColour = freezeAmount > 0.5f ? juce::Colour(0xffd0ac67) : juce::Colour(0xff3e4d5c);
        g.setColour(lampColour.withAlpha(0.35f));
        g.fillEllipse(lampArea.expanded(4.0f));
        g.setColour(lampColour);
        g.fillEllipse(lampArea);

        auto meters = inner.removeFromTop(inner.getHeight() - 66.0f);
        const float gap = 14.0f;
        const float barWidth = (meters.getWidth() - gap) * 0.5f;
        auto inBar = juce::Rectangle<float>(meters.getX(), meters.getY(), barWidth, meters.getHeight());
        auto outBar = juce::Rectangle<float>(meters.getX() + barWidth + gap, meters.getY(), barWidth, meters.getHeight());
        drawBar(g, inBar, inputLevel, "IN");
        drawBar(g, outBar, outputLevel, "OUT");

        auto footer = inner;
        auto stateBadge = footer.removeFromTop(22.0f).withTrimmedRight(18);
        g.setColour(freezeAmount > 0.5f ? brass : juce::Colour(0xff8391a1));
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(freezeAmount > 0.5f ? "FREEZE HOLD" : "FREEZE READY",
                   stateBadge.toNearestInt(), juce::Justification::centredLeft, false);

        g.setColour(juce::Colour(0xff8f9cac));
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::plain)));
        g.drawText("Input / Output", footer.toNearestInt(), juce::Justification::centredLeft, false);
    }

private:
    static void drawBar(juce::Graphics& g, juce::Rectangle<float> bounds, float level, const juce::String& label)
    {
        drawPlate(g, bounds, juce::Colour(0xff0d1115), 6.0f);
        auto inner = bounds.reduced(6.0f, 8.0f);
        const int stepCount = 10;

        for (int i = 1; i < stepCount; ++i)
        {
            const float y = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(stepCount),
                                       inner.getBottom(), inner.getY());
            g.setColour(juce::Colour(0x442ca0c0));
            g.drawHorizontalLine(static_cast<int>(y), inner.getX(), inner.getRight());
        }

        auto fill = inner;
        fill.setY(inner.getBottom() - inner.getHeight() * level);
        juce::ColourGradient meterFill(juce::Colour(0xff2fb37d), fill.getCentreX(), fill.getBottom(),
                                       juce::Colour(0xffd3a85f), fill.getCentreX(), fill.getY(), false);
        if (level > 0.82f)
            meterFill.addColour(0.0, juce::Colour(0xffd55c4f));
        g.setGradientFill(meterFill);
        g.fillRoundedRectangle(fill, 3.0f);

        auto labelArea = bounds.removeFromBottom(18.0f);
        g.setColour(warmGrey);
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(label, labelArea.toNearestInt(), juce::Justification::centred, false);
    }

    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    float freezeAmount = 0.0f;
};

class BloomVerbAudioProcessorEditor::DisplayPanel final : public juce::Component
{
public:
    explicit DisplayPanel(juce::AudioProcessorValueTreeState& state) : apvts(state)
    {
        setOpaque(true);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(4.0f);
        drawPlate(g, area, juce::Colour(0xff191613), 14.0f);
        drawScrews(g, area);

        auto inner = area.reduced(14.0f, 14.0f);
        auto titleRow = inner.removeFromTop(30.0f);
        auto titleBadge = titleRow.removeFromLeft(120.0f);
        drawPlate(g, titleBadge, juce::Colour(0xff26231f), 5.0f);
        g.setColour(warmGrey);
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.drawText("Room Field", titleBadge.toNearestInt(), juce::Justification::centred, false);

        g.setColour(juce::Colour(0xff89a4bb));
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::plain)));
        g.drawText(readTypeName().toUpperCase(), titleRow.toNearestInt(), juce::Justification::centredRight, false);

        auto screen = inner.removeFromTop(inner.getHeight() - 38.0f);
        juce::ColourGradient screenFill(juce::Colour(0xff071219), screen.getX(), screen.getY(),
                                        juce::Colour(0xff0c1a20), screen.getRight(), screen.getBottom(), false);
        g.setGradientFill(screenFill);
        g.fillRoundedRectangle(screen, 8.0f);
        g.setColour(juce::Colour(0x663ab2d1));
        g.drawRoundedRectangle(screen, 8.0f, 1.0f);

        auto viewport = screen.reduced(12.0f, 12.0f);
        drawGrid(g, viewport);
        drawRoom(g, viewport);

        auto footer = inner;
        auto statArea = footer.toNearestInt();
        g.setColour(juce::Colour(0xff6bb6d1));
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.drawText(buildFooterText(), statArea, juce::Justification::centredLeft, false);
    }

private:
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> viewport) const
    {
        g.setColour(juce::Colour(0x222bc7ea));
        for (int i = 1; i < 8; ++i)
        {
            const float x = juce::jmap(static_cast<float>(i), 0.0f, 8.0f, viewport.getX(), viewport.getRight());
            g.drawVerticalLine(static_cast<int>(x), viewport.getY(), viewport.getBottom());
        }

        for (int i = 1; i < 6; ++i)
        {
            const float y = juce::jmap(static_cast<float>(i), 0.0f, 6.0f, viewport.getY(), viewport.getBottom());
            g.drawHorizontalLine(static_cast<int>(y), viewport.getX(), viewport.getRight());
        }
    }

    void drawRoom(juce::Graphics& g, juce::Rectangle<float> viewport) const
    {
        using namespace bloomverb::params;

        const float size = getValue(apvts, IDs::size);
        const float distance = getValue(apvts, IDs::distance);
        const float width = juce::jlimit(0.0f, 1.0f, getValue(apvts, IDs::width) * 0.5f);
        const float early = getValue(apvts, IDs::early);
        const float decay = juce::jlimit(0.0f, 1.0f, getValue(apvts, IDs::decaySeconds) / 20.0f);

        auto roomArea = viewport.reduced(14.0f, 10.0f);
        const float topInset = juce::jmap(size, 0.0f, 1.0f, roomArea.getWidth() * 0.22f, roomArea.getWidth() * 0.08f);
        const float bottomInset = juce::jmap(width, 0.0f, 1.0f, roomArea.getWidth() * 0.30f, roomArea.getWidth() * 0.06f);
        const float depth = juce::jmap(size, 0.0f, 1.0f, roomArea.getHeight() * 0.46f, roomArea.getHeight() * 0.74f);

        juce::Point<float> backLeft(roomArea.getX() + topInset, roomArea.getY() + 8.0f);
        juce::Point<float> backRight(roomArea.getRight() - topInset, roomArea.getY() + 8.0f);
        juce::Point<float> frontLeft(roomArea.getX() + bottomInset, roomArea.getY() + depth);
        juce::Point<float> frontRight(roomArea.getRight() - bottomInset, roomArea.getY() + depth);
        juce::Point<float> floorLeft(frontLeft.x + 28.0f, roomArea.getBottom() - 6.0f);
        juce::Point<float> floorRight(frontRight.x - 28.0f, roomArea.getBottom() - 6.0f);

        juce::Path wireframe;
        wireframe.startNewSubPath(backLeft);
        wireframe.lineTo(backRight);
        wireframe.lineTo(frontRight);
        wireframe.lineTo(frontLeft);
        wireframe.closeSubPath();
        wireframe.startNewSubPath(frontLeft);
        wireframe.lineTo(floorLeft);
        wireframe.lineTo(floorRight);
        wireframe.lineTo(frontRight);
        wireframe.startNewSubPath(backLeft);
        wireframe.lineTo(floorLeft);
        wireframe.startNewSubPath(backRight);
        wireframe.lineTo(floorRight);

        g.setColour(screenAccent.withAlpha(0.92f));
        g.strokePath(wireframe, juce::PathStrokeType(1.7f));

        const float listenerX = juce::jmap(distance, 0.0f, 1.0f, frontLeft.x + 18.0f, frontRight.x - 18.0f);
        const float listenerY = juce::jmap(distance, 0.0f, 1.0f, floorLeft.y - 10.0f, backLeft.y + 40.0f);
        const juce::Rectangle<float> listener(10.0f, 10.0f);
        g.setColour(juce::Colour(0xff4ed08b));
        g.fillEllipse(listener.withCentre({ listenerX, listenerY }));

        const float sourceX = juce::jmap(width, 0.0f, 1.0f, frontLeft.x + 30.0f, frontRight.x - 30.0f);
        const float sourceY = juce::jmap(decay, 0.0f, 1.0f, roomArea.getBottom() - 24.0f, roomArea.getCentreY());
        g.setColour(juce::Colour(0xffd86d68));
        g.fillEllipse(listener.withCentre({ sourceX, sourceY }));

        g.setColour(screenAccent.withAlpha(0.65f));
        for (int i = 0; i < 4; ++i)
        {
            const float t = (static_cast<float>(i) + 1.0f) / 5.0f;
            const float rx = juce::jmap(t, backLeft.x, frontLeft.x);
            const float ry = juce::jmap(t + early * 0.18f, backLeft.y + 18.0f, floorLeft.y - 16.0f);
            g.drawEllipse(rx, ry, 6.0f + i * 4.0f, 6.0f + i * 4.0f, 1.0f);
        }
    }

    juce::String readTypeName() const
    {
        if (const auto* choice = dynamic_cast<const juce::AudioParameterChoice*>(apvts.getParameter(bloomverb::params::IDs::type)))
            return choice->getCurrentChoiceName();
        return "Type";
    }

    juce::String buildFooterText() const
    {
        using namespace bloomverb::params;
        juce::String text;
        text << "Size " << juce::String(getValue(apvts, IDs::size), 2)
             << "   Dist " << juce::String(getValue(apvts, IDs::distance), 2)
             << "   Width " << juce::String(getValue(apvts, IDs::width), 2);
        return text;
    }

    juce::AudioProcessorValueTreeState& apvts;
};

class BloomVerbAudioProcessorEditor::ParameterModule final : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    ParameterModule(juce::AudioProcessorValueTreeState& state,
                    juce::String titleText,
                    int rowsIn,
                    int columnsIn,
                    juce::Colour accentIn)
        : apvts(state), title(std::move(titleText)), rows(rowsIn), columns(columnsIn), accent(accentIn)
    {
        moduleTint = accent.interpolatedWith(juce::Colour(0xffc8c0b5), 0.78f)
                         .withMultipliedSaturation(0.34f)
                         .withMultipliedBrightness(0.82f);
        setOpaque(true);
    }

    void addKnob(const juce::String& paramID,
                 const juce::String& name,
                 int row,
                 int column,
                 juce::Colour knobAccent = {})
    {
        auto control = std::make_unique<SliderControl>();
        control->label.setText(name, juce::dontSendNotification);
        control->label.setJustificationType(juce::Justification::centred);
        control->label.setColour(juce::Label::textColourId, juce::Colour(0xfff1e8d7));
        control->label.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
        control->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        control->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
        control->slider.setPopupDisplayEnabled(true, false, this);
        control->slider.setColour(juce::Slider::rotarySliderFillColourId,
                                  knobAccent.isTransparent() ? accent : knobAccent);
        control->row = row;
        control->column = column;
        control->attachment = std::make_unique<SliderAttachment>(apvts, paramID, control->slider);

        addAndMakeVisible(control->label);
        addAndMakeVisible(control->slider);
        sliders.push_back(std::move(control));
    }

    void addToggle(const juce::String& paramID,
                   const juce::String& name,
                   int row,
                   int column)
    {
        auto control = std::make_unique<ToggleControl>();
        control->button.setButtonText(name);
        control->button.setColour(juce::ToggleButton::textColourId, warmGrey);
        control->row = row;
        control->column = column;
        control->attachment = std::make_unique<ButtonAttachment>(apvts, paramID, control->button);
        addAndMakeVisible(control->button);
        toggles.push_back(std::move(control));
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(2.0f);
        drawPlate(g, area, moduleTint, 11.0f);
        drawScrews(g, area);

        auto nameplate = area.reduced(8.0f, 8.0f).removeFromTop(24.0f);
        drawPlate(g, nameplate, juce::Colour(0xff2b2824), 5.0f);
        g.setColour(juce::Colour(0xfff3e6cf));
        g.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
        g.drawText(title, nameplate.toNearestInt(), juce::Justification::centred, false);

        auto accentLine = area.reduced(10.0f, 10.0f).removeFromTop(2.0f).withWidth(area.getWidth() - 20.0f);
        accentLine.setY(nameplate.getBottom() + 6.0f);
        g.setColour(accent.withAlpha(0.75f));
        g.fillRoundedRectangle(accentLine, 1.2f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10, 10);
        area.removeFromTop(33);

        const int safeRows = juce::jmax(1, rows);
        const int safeColumns = juce::jmax(1, columns);
        const int cellWidth = area.getWidth() / safeColumns;
        const int cellHeight = area.getHeight() / safeRows;

        for (const auto& knob : sliders)
        {
            auto cell = juce::Rectangle<int>(area.getX() + knob->column * cellWidth,
                                             area.getY() + knob->row * cellHeight,
                                             cellWidth,
                                             cellHeight).reduced(3, 2);
            auto labelArea = cell.removeFromTop(15);
            const int knobSize = juce::jlimit(56, 92, juce::jmin(cell.getWidth() - 4, cell.getHeight() - 8));
            auto knobArea = juce::Rectangle<int>(knobSize, knobSize).withCentre(cell.getCentre());
            knobArea.setY(labelArea.getBottom() + juce::jmax(0, (cell.getHeight() - knobSize) / 2));
            knob->label.setBounds(labelArea);
            knob->slider.setBounds(knobArea);
        }

        for (const auto& toggle : toggles)
        {
            auto cell = juce::Rectangle<int>(area.getX() + toggle->column * cellWidth,
                                             area.getY() + toggle->row * cellHeight,
                                             cellWidth,
                                             cellHeight).reduced(4, 8);
            toggle->button.setBounds(cell.withSizeKeepingCentre(juce::jmin(150, cell.getWidth()), 30));
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
    juce::String title;
    int rows = 1;
    int columns = 1;
    juce::Colour accent;
    juce::Colour moduleTint;
    std::vector<std::unique_ptr<SliderControl>> sliders;
    std::vector<std::unique_ptr<ToggleControl>> toggles;
};

BloomVerbAudioProcessorEditor::BloomVerbAudioProcessorEditor(BloomVerbAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), apvts(p.getAPVTS()),
      lookAndFeel(std::make_unique<ProductionLookAndFeel>()),
      levelMeter(std::make_unique<LevelMeter>()),
      displayPanel(std::make_unique<DisplayPanel>(apvts))
{
    setLookAndFeel(lookAndFeel.get());
    setSize(1480, 860);
    setResizable(false, false);

    titleLabel.setText("BloomVerb", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff2e4cc));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("console scaffold / assetless placeholder", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffb8afa3));
    subtitleLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::plain)));
    addAndMakeVisible(subtitleLabel);

    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff95b3c7));
    statusLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(statusLabel);

    setupPresetControls();

    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    typeLabel.setColour(juce::Label::textColourId, warmGrey);
    addAndMakeVisible(typeLabel);

    typeBox.addItemList(bloomverb::params::getTypeChoices(), 1);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, bloomverb::params::IDs::type, typeBox);
    addAndMakeVisible(typeBox);

    outputFaderLabel.setText("Output", juce::dontSendNotification);
    outputFaderLabel.setJustificationType(juce::Justification::centred);
    outputFaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff0e1c7));
    outputFaderLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(outputFaderLabel);

    outputFader.setSliderStyle(juce::Slider::LinearVertical);
    outputFader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    outputFader.setPopupDisplayEnabled(true, false, this);
    outputFader.setColour(juce::Slider::trackColourId, juce::Colour(0xff8fcde0));
    outputFader.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd9ccb8));
    outputFaderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, bloomverb::params::IDs::outputDb, outputFader);
    addAndMakeVisible(outputFader);

#if JucePlugin_Build_Standalone
    loadFileButton.setButtonText("Load");
    loadFileButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select audio file",
                                                          juce::File{},
                                                          "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");

        const auto chooserFlags = juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
        {
            const auto selectedFile = chooser.getResult();
            if (selectedFile.existsAsFile())
                processor.loadStandalonePlaybackFile(selectedFile);

            refreshStandalonePlaybackState();
        });
    };
    addAndMakeVisible(loadFileButton);

    playFileButton.setButtonText("Play");
    playFileButton.onClick = [this]
    {
        processor.setStandalonePlaybackActive(!processor.isStandalonePlaybackActive());
        refreshStandalonePlaybackState();
    };
    addAndMakeVisible(playFileButton);

    loopFileToggle.setButtonText("Loop");
    loopFileToggle.onClick = [this]
    {
        processor.setStandalonePlaybackLooping(loopFileToggle.getToggleState());
        refreshStandalonePlaybackState();
    };
    addAndMakeVisible(loopFileToggle);

    fileStatusLabel.setJustificationType(juce::Justification::centredLeft);
    fileStatusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaca69c));
    fileStatusLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::plain)));
    addAndMakeVisible(fileStatusLabel);
#endif

    setupModules();
    addAndMakeVisible(*displayPanel);
    addAndMakeVisible(*levelMeter);

#if JucePlugin_Build_Standalone
    refreshStandalonePlaybackState();
#endif
    resized();
    repaint();
    startTimerHz(24);
}

void BloomVerbAudioProcessorEditor::setupPresetControls()
{
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    presetLabel.setColour(juce::Label::textColourId, warmGrey);
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

void BloomVerbAudioProcessorEditor::setupModules()
{
    spaceModule = std::make_unique<ParameterModule>(apvts, "Space", 3, 2, stripWarm);
    spaceModule->addKnob(bloomverb::params::IDs::size, "Size", 0, 0, brass);
    spaceModule->addKnob(bloomverb::params::IDs::decaySeconds, "Decay", 0, 1, juce::Colour(0xff9aa7bf));
    spaceModule->addKnob(bloomverb::params::IDs::preDelayMs, "PreDelay", 1, 0, juce::Colour(0xff88acba));
    spaceModule->addKnob(bloomverb::params::IDs::mix, "Mix", 1, 1, redAccent);
    spaceModule->addKnob(bloomverb::params::IDs::distance, "Distance", 2, 0, juce::Colour(0xff9ba38a));
    addAndMakeVisible(*spaceModule);

    bloomModule = std::make_unique<ParameterModule>(apvts, "Bloom", 4, 1, stripCool);
    bloomModule->addKnob(bloomverb::params::IDs::motion, "Motion", 0, 0, cyanAccent);
    bloomModule->addKnob(bloomverb::params::IDs::texture, "Texture", 1, 0, mintAccent);
    bloomModule->addKnob(bloomverb::params::IDs::swell, "Swell", 2, 0, juce::Colour(0xff7cb2cf));
    bloomModule->addKnob(bloomverb::params::IDs::bloomAmount, "Bloom", 3, 0, redAccent);
    addAndMakeVisible(*bloomModule);

    characterModule = std::make_unique<ParameterModule>(apvts, "Character", 3, 2, stripNeutral);
    characterModule->addKnob(bloomverb::params::IDs::dynamic, "Dynamic", 0, 0, juce::Colour(0xff95bb9f));
    characterModule->addKnob(bloomverb::params::IDs::harmonic, "Harmonic", 0, 1, juce::Colour(0xff8db4c0));
    characterModule->addKnob(bloomverb::params::IDs::warp, "Warp", 1, 0, juce::Colour(0xffb88d78));
    characterModule->addKnob(bloomverb::params::IDs::damping, "Damping", 1, 1, juce::Colour(0xff8aa38f));
    characterModule->addKnob(bloomverb::params::IDs::width, "Width", 2, 0, cyanAccent);
    characterModule->addKnob(bloomverb::params::IDs::early, "Early", 2, 1, juce::Colour(0xffb69872));
    addAndMakeVisible(*characterModule);

    sculptModule = std::make_unique<ParameterModule>(apvts, "Tone / Filters", 4, 1, stripCool);
    sculptModule->addKnob(bloomverb::params::IDs::diffusion, "Diffusion", 0, 0, juce::Colour(0xff9ca3b8));
    sculptModule->addKnob(bloomverb::params::IDs::tone, "Tone", 1, 0, juce::Colour(0xffc7b4a1));
    sculptModule->addKnob(bloomverb::params::IDs::lowCutHz, "Low Cut", 2, 0, juce::Colour(0xff8ca3ac));
    sculptModule->addKnob(bloomverb::params::IDs::highCutHz, "High Cut", 3, 0, juce::Colour(0xff95afbf));
    addAndMakeVisible(*sculptModule);

    outputModule = std::make_unique<ParameterModule>(apvts, "Mod / Utility", 3, 2, stripWarm);
    outputModule->addKnob(bloomverb::params::IDs::modRateHz, "Mod Rate", 0, 0, juce::Colour(0xff87aec0));
    outputModule->addKnob(bloomverb::params::IDs::modDepth, "Mod Depth", 0, 1, juce::Colour(0xff75b5c6));
    outputModule->addKnob(bloomverb::params::IDs::duckAmount, "Duck", 1, 0, juce::Colour(0xffb68a77));
    outputModule->addKnob(bloomverb::params::IDs::transientPreserve, "Transient", 1, 1, juce::Colour(0xff8ab091));
    outputModule->addToggle(bloomverb::params::IDs::freeze, "Freeze Hold", 2, 0);
    addAndMakeVisible(*outputModule);
}

BloomVerbAudioProcessorEditor::~BloomVerbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void BloomVerbAudioProcessorEditor::refreshStandalonePlaybackState()
{
#if JucePlugin_Build_Standalone
    const bool hasFile = processor.hasStandalonePlaybackFile();
    playFileButton.setEnabled(hasFile);
    playFileButton.setButtonText(processor.isStandalonePlaybackActive() ? "Stop" : "Play");
    loopFileToggle.setToggleState(processor.isStandalonePlaybackLooping(), juce::dontSendNotification);
    fileStatusLabel.setText(processor.getStandalonePlaybackFileLabel() + "  |  "
                                + processor.getStandalonePlaybackStatusText(),
                            juce::dontSendNotification);
#endif
}

void BloomVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient background(chassisInner, 0.0f, 0.0f,
                                    juce::Colour(0xff050608), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(background);
    g.fillAll();

    auto chassis = getLocalBounds().reduced(8).toFloat();
    juce::ColourGradient chassisFill(chassisOuter.brighter(0.08f), chassis.getCentreX(), chassis.getY(),
                                     chassisOuter.darker(0.25f), chassis.getCentreX(), chassis.getBottom(), false);
    g.setGradientFill(chassisFill);
    g.fillRoundedRectangle(chassis, 18.0f);
    g.setColour(juce::Colour(0xff87725b));
    g.drawRoundedRectangle(chassis, 18.0f, 1.4f);
    drawScrews(g, chassis);

    auto inner = chassis.reduced(16.0f, 16.0f);
    auto topStrip = inner.removeFromTop(118.0f);
    drawPlate(g, topStrip, juce::Colour(0xff211d18), 14.0f);

    auto contentBed = inner.reduced(0.0f, 1.0f);
    auto masterRail = contentBed.removeFromRight(332.0f);
    drawPlate(g, masterRail, juce::Colour(0xff171513), 14.0f);
    contentBed.removeFromRight(10.0f);

    drawPlate(g, contentBed, juce::Colour(0xff14110f), 14.0f);

    const int stripGap = 8;
    const int stripCount = 5;
    const float stripWidth = (contentBed.getWidth() - static_cast<float>(stripGap * (stripCount - 1)))
                             / static_cast<float>(stripCount);
    for (int i = 0; i < stripCount; ++i)
    {
        const float x = contentBed.getX() + i * (stripWidth + static_cast<float>(stripGap));
        auto stripArea = juce::Rectangle<float>(x, contentBed.getY(), stripWidth, contentBed.getHeight()).reduced(2.0f, 2.0f);
        drawPlate(g, stripArea, (i % 2 == 0 ? juce::Colour(0xff15130f) : juce::Colour(0xff171619)), 11.0f);
    }

    auto faderWell = masterRail.reduced(10.0f, 10.0f).removeFromRight(82.0f);
    drawPlate(g, faderWell, juce::Colour(0xff121417), 9.0f);
}

void BloomVerbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(22, 20);
    auto header = area.removeFromTop(104);
    auto titleRow = header.removeFromTop(38);

    titleLabel.setBounds(titleRow.removeFromLeft(280));
    statusLabel.setBounds(titleRow.removeFromRight(320));

    auto infoRow = header.removeFromTop(22);
    subtitleLabel.setBounds(infoRow.removeFromLeft(280));
    typeLabel.setBounds(infoRow.removeFromLeft(34));
    infoRow.removeFromLeft(6);
    typeBox.setBounds(infoRow.removeFromLeft(192));

    auto presetRow = header.removeFromTop(30);
    presetLabel.setBounds(presetRow.removeFromLeft(42));
    presetPrevButton.setBounds(presetRow.removeFromLeft(30));
    presetRow.removeFromLeft(4);
    presetBox.setBounds(presetRow.removeFromLeft(244));
    presetRow.removeFromLeft(4);
    presetNextButton.setBounds(presetRow.removeFromLeft(30));

#if JucePlugin_Build_Standalone
    presetRow.removeFromLeft(12);
    loadFileButton.setBounds(presetRow.removeFromLeft(64));
    presetRow.removeFromLeft(4);
    playFileButton.setBounds(presetRow.removeFromLeft(64));
    presetRow.removeFromLeft(6);
    loopFileToggle.setBounds(presetRow.removeFromLeft(94));
    presetRow.removeFromLeft(10);
    fileStatusLabel.setBounds(presetRow);
#endif

    const int sectionGap = 10;
    auto masterRail = area.removeFromRight(332);
    area.removeFromRight(sectionGap);

    const int stripGap = 8;
    const int stripCount = 5;
    const int stripWidth = (area.getWidth() - stripGap * (stripCount - 1)) / stripCount;

    std::array<juce::Rectangle<int>, 5> strips {};
    for (int i = 0; i < stripCount; ++i)
    {
        strips[static_cast<std::size_t>(i)] = area.removeFromLeft(stripWidth);
        if (i < stripCount - 1)
            area.removeFromLeft(stripGap);
    }

    if (spaceModule != nullptr)
        spaceModule->setBounds(strips[0]);
    if (bloomModule != nullptr)
        bloomModule->setBounds(strips[1]);
    if (characterModule != nullptr)
        characterModule->setBounds(strips[2]);
    if (sculptModule != nullptr)
        sculptModule->setBounds(strips[3]);
    if (outputModule != nullptr)
        outputModule->setBounds(strips[4]);

    auto screenArea = masterRail.removeFromTop(250).reduced(2, 0);
    if (displayPanel != nullptr)
        displayPanel->setBounds(screenArea);

    masterRail.removeFromTop(8);
    auto meterSection = masterRail;
    auto faderLane = meterSection.removeFromRight(82).reduced(2, 0);
    if (levelMeter != nullptr)
        levelMeter->setBounds(meterSection);

    auto faderInner = faderLane.reduced(6, 8);
    outputFaderLabel.setBounds(faderInner.removeFromTop(20));
    faderInner.removeFromTop(4);
    outputFader.setBounds(faderInner);
}

void BloomVerbAudioProcessorEditor::timerCallback()
{
    const float inputLevel = processor.getInputMeterLevel();
    const float outputLevel = processor.getOutputMeterLevel();
    const float freezeAmount = processor.getFreezeVisualAmount();

    if (levelMeter != nullptr)
        levelMeter->setLevels(inputLevel, outputLevel, freezeAmount);

    if (displayPanel != nullptr)
        displayPanel->repaint();

#if JucePlugin_Build_Standalone
    refreshStandalonePlaybackState();
#endif

    const auto presetCount = processor.getPresetNames().size();
    const auto presetIndex = processor.getCurrentPresetIndex();
    juce::String statusText;

    if (freezeAmount > 0.5f)
        statusText << "Freeze hold armed";
    else if (outputLevel > 0.85f)
        statusText << "Output hot";
    else if (inputLevel < 0.01f && outputLevel < 0.01f)
        statusText << "Input silent";
    else
        statusText << "Ready";

    statusText << "  |  ";
    if (presetCount > 0 && presetIndex >= 0)
        statusText << "Preset " << (presetIndex + 1) << "/" << presetCount;
    else
        statusText << "No presets";

    if (statusLabel.getText() != statusText)
        statusLabel.setText(statusText, juce::dontSendNotification);
}
