#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Dark/modern LookAndFeel for CHORwerk v2.
 */
class CHORwerkLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CHORwerkLookAndFeel();
    ~CHORwerkLookAndFeel() override = default;

    struct Colors
    {
        static constexpr juce::uint32 background    = 0xff080e18;
        static constexpr juce::uint32 surface       = 0xff0c1520;
        static constexpr juce::uint32 surfaceLight  = 0xff152030;
        static constexpr juce::uint32 border        = 0xff1a2a3a;
        static constexpr juce::uint32 textPrimary   = 0xffe2e8f0;
        static constexpr juce::uint32 textSecondary = 0xff8b9caa;
        static constexpr juce::uint32 textDim       = 0xff475569;

        static constexpr juce::uint32 voices   = 0xff6ee7b7;
        static constexpr juce::uint32 rate     = 0xff38bdf8;
        static constexpr juce::uint32 delay    = 0xffa78bfa;
        static constexpr juce::uint32 filter   = 0xfff472b6;
        static constexpr juce::uint32 feedback = 0xfffb923c;
        static constexpr juce::uint32 stepSeq  = 0xfffacc15;
        static constexpr juce::uint32 envelope = 0xff34d399;
        static constexpr juce::uint32 lfo      = 0xffa78bfa;
    };

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    juce::Font getPopupMenuFont() override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;

    void setAccentColour (juce::Colour c) { currentAccent = c; }
    void setScale (float s) { currentScale = s; }

private:
    juce::Colour currentAccent { Colors::voices };
    float currentScale = 1.0f;
};
