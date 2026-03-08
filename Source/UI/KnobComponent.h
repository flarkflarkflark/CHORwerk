#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

/**
 * Rotary knob with label above, value readout below.
 * Accepts an accent colour for per-section theming.
 * Wraps juce::Slider + APVTS attachment.
 */
class KnobComponent : public juce::Component
{
public:
    KnobComponent (const juce::String& labelText,
                   const juce::String& suffix = "",
                   juce::Colour accent = juce::Colour (CharsiesiLookAndFeel::Colors::voices),
                   bool bipolar = false)
        : label (labelText), accentColour (accent), unitSuffix (suffix), isBipolar (bipolar)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        slider.setPopupDisplayEnabled (false, false, nullptr);
        slider.setDoubleClickReturnValue (true, bipolar ? 0.0 : 0.5);
        slider.setColour (juce::Slider::thumbColourId, accent);
        addAndMakeVisible (slider);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    }

    void setAccentColour (juce::Colour c) { accentColour = c; }

    void resized() override
    {
        auto b = getLocalBounds();
        int labelH = 14;
        int valueH = 14;
        int knobSize = juce::jmin (b.getWidth(), b.getHeight() - labelH - valueH);

        // Label at top
        labelBounds = b.removeFromTop (labelH);

        // Value at bottom
        valueBounds = b.removeFromBottom (valueH);

        // Knob centered in remaining space
        auto knobArea = b.withSizeKeepingCentre (knobSize, knobSize);
        slider.setBounds (knobArea);
    }

    void paint (juce::Graphics& g) override
    {
        // Label
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::textSecondary));
        g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
        g.drawText (label.toUpperCase(), labelBounds, juce::Justification::centred);

        // Value readout
        auto value = slider.getValue();
        juce::String valueStr;
        if (std::abs (value) < 10.0)
            valueStr = juce::String (value, 2);
        else if (std::abs (value) < 100.0)
            valueStr = juce::String (value, 1);
        else
            valueStr = juce::String (static_cast<int> (value));

        if (unitSuffix.isNotEmpty())
            valueStr += unitSuffix;

        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::textPrimary).withAlpha (0.85f));
        g.setFont (juce::Font ("JetBrains Mono", 10.0f, juce::Font::plain));
        g.drawText (valueStr, valueBounds, juce::Justification::centred);
    }

    void parentHierarchyChanged() override
    {
        // Set accent colour on LookAndFeel before paint
        if (auto* laf = dynamic_cast<CharsiesiLookAndFeel*> (&getLookAndFeel()))
            laf->setAccentColour (accentColour);
    }

    /** Called before drawRotarySlider so the LookAndFeel knows what colour to use. */
    void lookAndFeelChanged() override {}

    juce::Slider& getSlider() { return slider; }

    // Override to push accent colour before each paint
    void paintOverChildren (juce::Graphics&) override
    {
        if (auto* laf = dynamic_cast<CharsiesiLookAndFeel*> (&getLookAndFeel()))
            laf->setAccentColour (accentColour);
    }

private:
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::String label;
    juce::Colour accentColour;
    juce::String unitSuffix;
    bool isBipolar;

    juce::Rectangle<int> labelBounds, valueBounds;

    // Ensure accent colour is set before slider draws
    struct AccentSetter : public juce::Slider::LookAndFeelMethods
    {
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobComponent)
};

/**
 * Toggle switch with APVTS attachment, styled per section.
 */
class ToggleComponent : public juce::Component
{
public:
    ToggleComponent (const juce::String& labelText,
                     juce::Colour accent = juce::Colour (CharsiesiLookAndFeel::Colors::voices))
        : accentColour (accent)
    {
        toggle.setButtonText (labelText);
        addAndMakeVisible (toggle);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, toggle);
    }

    void resized() override { toggle.setBounds (getLocalBounds()); }

    void paintOverChildren (juce::Graphics&) override
    {
        if (auto* laf = dynamic_cast<CharsiesiLookAndFeel*> (&getLookAndFeel()))
            laf->setAccentColour (accentColour);
    }

    juce::ToggleButton& getToggle() { return toggle; }

private:
    juce::ToggleButton toggle;
    juce::Colour accentColour;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleComponent)
};

/**
 * ComboBox with APVTS attachment.
 */
class DropdownComponent : public juce::Component
{
public:
    DropdownComponent (const juce::String& labelText,
                       juce::Colour accent = juce::Colour (CharsiesiLookAndFeel::Colors::voices))
        : label (labelText), accentColour (accent)
    {
        addAndMakeVisible (box);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, box);
    }

    juce::ComboBox& getBox() { return box; }

    void resized() override
    {
        auto b = getLocalBounds();
        if (label.isNotEmpty())
        {
            labelBounds = b.removeFromTop (14);
            b.removeFromTop (2);
        }
        box.setBounds (b);
    }

    void paint (juce::Graphics& g) override
    {
        if (label.isNotEmpty())
        {
            g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::textSecondary));
            g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
            g.drawText (label.toUpperCase(), labelBounds, juce::Justification::centredLeft);
        }
    }

private:
    juce::ComboBox box;
    juce::String label;
    juce::Colour accentColour;
    juce::Rectangle<int> labelBounds;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DropdownComponent)
};
