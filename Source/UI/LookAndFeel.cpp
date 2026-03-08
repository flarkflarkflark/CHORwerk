#include "LookAndFeel.h"

CharsiesiLookAndFeel::CharsiesiLookAndFeel()
{
    // Set default colours
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (Colors::border));
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (Colors::textPrimary));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (Colors::surface));

    setColour (juce::ComboBox::backgroundColourId, juce::Colour (Colors::surface));
    setColour (juce::ComboBox::textColourId, juce::Colour (Colors::textPrimary));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (Colors::border));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (Colors::textSecondary));

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (Colors::surface));
    setColour (juce::PopupMenu::textColourId, juce::Colour (Colors::textPrimary));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (Colors::surfaceLight));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (Colors::textPrimary));

    setColour (juce::Label::textColourId, juce::Colour (Colors::textSecondary));
}

void CharsiesiLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider)
{
    // Read accent colour from the slider's thumb colour (set per KnobComponent)
    auto accent = slider.findColour (juce::Slider::thumbColourId);
    if (accent.isTransparent())
        accent = currentAccent;

    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Track background
    juce::Path bgTrack;
    bgTrack.addCentredArc (centreX, centreY, radius - 2, radius - 2,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (Colors::surfaceLight));
    g.strokePath (bgTrack, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // Active arc
    if (sliderPos > 0.01f)
    {
        juce::Path activeTrack;
        activeTrack.addCentredArc (centreX, centreY, radius - 2, radius - 2,
                                   0.0f, rotaryStartAngle, angle, true);
        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (activeTrack, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
    }

    // Center dot
    g.setColour (juce::Colour (0xff1a2332));
    g.fillEllipse (centreX - 3, centreY - 3, 6, 6);
    g.setColour (juce::Colour (0xff2a3a4a));
    g.drawEllipse (centreX - 3, centreY - 3, 6, 6, 1.0f);

    // Pointer line
    juce::Path pointer;
    auto pointerLength = radius - 6.0f;
    auto pointerThickness = 2.0f;
    pointer.addRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength - 4.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (juce::Colour (Colors::textPrimary));
    g.fillPath (pointer);

    // Glow dot at pointer tip
    auto tipX = centreX + (radius - 8.0f) * std::sin (angle);
    auto tipY = centreY - (radius - 8.0f) * std::cos (angle);
    g.setColour (accent.withAlpha (0.6f));
    g.fillEllipse (tipX - 2, tipY - 2, 4, 4);
}

void CharsiesiLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                              bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    bool isOn = button.getToggleState();

    // Background
    if (isOn)
    {
        g.setColour (currentAccent.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (currentAccent.withAlpha (0.25f));
    }
    else
    {
        g.setColour (juce::Colour (Colors::border));
    }
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Checkbox
    auto checkBounds = bounds.reduced (6.0f, (bounds.getHeight() - 10.0f) * 0.5f);
    checkBounds.setWidth (10.0f);

    g.setColour (isOn ? currentAccent : juce::Colour (0xff0f1a24));
    g.fillRoundedRectangle (checkBounds, 2.0f);
    g.setColour (isOn ? currentAccent : juce::Colour (0xff334155));
    g.drawRoundedRectangle (checkBounds, 2.0f, 1.0f);

    if (isOn)
    {
        g.setColour (currentAccent.withAlpha (0.3f));
        g.drawRoundedRectangle (checkBounds.expanded (1), 3.0f, 2.0f);
    }

    // Label
    g.setColour (isOn ? juce::Colour (Colors::textPrimary) : juce::Colour (0xff64748b));
    auto textBounds = bounds;
    textBounds.removeFromLeft (20.0f);
    g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
    g.drawText (button.getButtonText().toUpperCase(), textBounds, juce::Justification::centredLeft);
}

void CharsiesiLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                          int, int, int, int, juce::ComboBox&)
{
    auto bounds = juce::Rectangle<float> (0, 0, static_cast<float>(width), static_cast<float>(height));

    g.setColour (juce::Colour (Colors::surface));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (juce::Colour (Colors::border));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Arrow
    auto arrowBounds = bounds.removeFromRight (20.0f).reduced (6.0f);
    juce::Path arrow;
    arrow.addTriangle (arrowBounds.getX(), arrowBounds.getCentreY() - 2,
                       arrowBounds.getRight(), arrowBounds.getCentreY() - 2,
                       arrowBounds.getCentreX(), arrowBounds.getCentreY() + 3);
    g.setColour (juce::Colour (Colors::textSecondary));
    g.fillPath (arrow);
}

void CharsiesiLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float, float,
                                              juce::Slider::SliderStyle style, juce::Slider&)
{
    if (style != juce::Slider::LinearHorizontal)
        return;

    auto bounds = juce::Rectangle<float> (static_cast<float>(x), static_cast<float>(y),
                                           static_cast<float>(width), static_cast<float>(height));
    auto trackBounds = bounds.reduced (0, bounds.getHeight() * 0.35f);

    // Track background
    g.setColour (juce::Colour (Colors::surface));
    g.fillRoundedRectangle (trackBounds, 3.0f);
    g.setColour (juce::Colour (Colors::border));
    g.drawRoundedRectangle (trackBounds, 3.0f, 1.0f);

    // Active fill
    auto fillWidth = sliderPos - static_cast<float>(x);
    if (fillWidth > 1.0f)
    {
        auto fillBounds = trackBounds;
        fillBounds.setWidth (fillWidth);
        g.setGradientFill (juce::ColourGradient (
            currentAccent.withAlpha (0.5f), fillBounds.getX(), 0,
            currentAccent, fillBounds.getRight(), 0, false));
        g.fillRoundedRectangle (fillBounds, 3.0f);
    }
}
