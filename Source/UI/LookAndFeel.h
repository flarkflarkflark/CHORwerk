#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Dark/modern LookAndFeel for Charsiesis v2.
 * Color scheme: deep navy background, mint/teal accents,
 * with per-section color coding.
 */
class CharsiesiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CharsiesiLookAndFeel();
    ~CharsiesiLookAndFeel() override = default;

    // Colour palette
    struct Colors
    {
        static constexpr juce::uint32 background    = 0xff080e18;
        static constexpr juce::uint32 surface       = 0xff0c1520;
        static constexpr juce::uint32 surfaceLight  = 0xff152030;
        static constexpr juce::uint32 border        = 0xff1a2a3a;
        static constexpr juce::uint32 textPrimary   = 0xffe2e8f0;
        static constexpr juce::uint32 textSecondary = 0xff8b9caa;
        static constexpr juce::uint32 textDim       = 0xff475569;

        // Section accent colors
        static constexpr juce::uint32 voices   = 0xff6ee7b7;  // Mint green
        static constexpr juce::uint32 rate     = 0xff38bdf8;  // Sky blue
        static constexpr juce::uint32 delay    = 0xffa78bfa;  // Violet
        static constexpr juce::uint32 filter   = 0xfff472b6;  // Pink
        static constexpr juce::uint32 feedback = 0xfffb923c;  // Orange
        static constexpr juce::uint32 stepSeq  = 0xfffacc15;  // Yellow
        static constexpr juce::uint32 envelope = 0xff34d399;  // Emerald
        static constexpr juce::uint32 lfo      = 0xffa78bfa;  // Violet (same as delay)
    };

    // Knob drawing
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    // Toggle button (checkbox style)
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    // ComboBox
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    // Linear slider (for horizontal sliders)
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    /** Set the accent color for the next component being drawn. */
    void setAccentColour (juce::Colour c) { currentAccent = c; }

private:
    juce::Colour currentAccent { Colors::voices };
};
