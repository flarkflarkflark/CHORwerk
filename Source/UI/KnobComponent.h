#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"
#include <functional>

/**
 * Rotary knob with relative scaling text and dynamic unit suffixes.
 */
class KnobComponent : public juce::Component,
                   public juce::Slider::Listener
{
public:
    KnobComponent (const juce::String& labelText,
                   const juce::String& suffix = "",
                   juce::Colour accent = juce::Colour (CHORwerkLookAndFeel::Colors::voices),
                   bool bipolar = false)
        : label (labelText), accentColour (accent), unitSuffix (suffix), isBipolar (bipolar)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        slider.setPopupDisplayEnabled (false, false, nullptr);
        slider.setDoubleClickReturnValue (true, bipolar ? 0.0 : 0.5);
        slider.setColour (juce::Slider::thumbColourId, accent);
        slider.addListener (this);
        addAndMakeVisible (slider);
    }

    ~KnobComponent() override { slider.removeListener (this); }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    }

    void setAccentColour (juce::Colour c) { accentColour = c; }
    void setTooltip (const juce::String& text) { slider.setTooltip (text); }
    
    /** Allows changing the suffix dynamically (e.g. for tempo sync). */
    void setSuffix (const juce::String& s) { if (unitSuffix != s) { unitSuffix = s; repaint(); } }
    
    /** Provide a lambda to compute suffix dynamically on every paint. */
    void setSuffixLambda (std::function<juce::String()> lambda) { suffixLambda = lambda; }

    void sliderValueChanged (juce::Slider*) override { repaint(); }

    void resized() override
    {
        auto b = getLocalBounds();
        float h = static_cast<float>(b.getHeight());
        int labelH = static_cast<int>(h * 0.2f);
        int valueH = static_cast<int>(h * 0.2f);
        int knobSize = juce::jmin (b.getWidth(), b.getHeight() - labelH - valueH);

        labelBounds = b.removeFromTop (labelH);
        valueBounds = b.removeFromBottom (valueH);
        slider.setBounds (b.withSizeKeepingCentre (knobSize, knobSize));
    }

    void paint (juce::Graphics& g) override
    {
        if (auto* laf = dynamic_cast<CHORwerkLookAndFeel*> (&getLookAndFeel()))
            laf->setAccentColour (accentColour);

        float h = static_cast<float>(getHeight());
        
        // Label
        g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::textSecondary));
        g.setFont (juce::Font ("JetBrains Mono", h * 0.16f, juce::Font::bold));
        
        juce::String cleanLabel = label;
        cleanLabel = cleanLabel.replace ("\xe2\x86\x92", ">");
        cleanLabel = cleanLabel.replace ("\xc3\xa2\xe2\x80\xa0\xe2\x84\xa2", ">");
            
        g.drawText (cleanLabel.toUpperCase(), labelBounds, juce::Justification::centred);

        // Value + Suffix
        auto value = slider.getValue();
        juce::String valueStr = slider.getTextFromValue (value);
        
        juce::String currentSuffix = suffixLambda ? suffixLambda() : unitSuffix;

        if (currentSuffix.isNotEmpty() && !valueStr.contains(currentSuffix))
            valueStr += " " + currentSuffix;

        g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::textPrimary).withAlpha (0.95f));
        g.setFont (juce::Font ("JetBrains Mono", h * 0.16f, juce::Font::plain));
        g.drawText (valueStr, valueBounds, juce::Justification::centred);
    }

    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String label;
    juce::Colour accentColour;
    juce::String unitSuffix;
    std::function<juce::String()> suffixLambda;
    bool isBipolar;
    juce::Rectangle<int> labelBounds, valueBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobComponent)
};

/**
 * Toggle switch with relative label.
 */
class ToggleComponent : public juce::Component
{
public:
    ToggleComponent (const juce::String& labelText,
                     juce::Colour accent = juce::Colour (CHORwerkLookAndFeel::Colors::voices),
                     const juce::String& checkboxText = "",
                     bool useTopLabel = true)
        : label (labelText), accentColour (accent), topLabel (useTopLabel)
    {
        toggle.setButtonText (checkboxText.isEmpty() && !useTopLabel ? labelText : checkboxText);
        addAndMakeVisible (toggle);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, toggle);
    }

    void setTooltip (const juce::String& text) { toggle.setTooltip (text); }

    void resized() override
    {
        auto b = getLocalBounds();
        if (topLabel && label.isNotEmpty())
        {
            labelBounds = b.removeFromTop (static_cast<int>(getHeight() * 0.45f));
            b.removeFromTop (2);
        }
        toggle.setBounds (b);
    }

    void paint (juce::Graphics& g) override
    {
        if (auto* laf = dynamic_cast<CHORwerkLookAndFeel*> (&getLookAndFeel()))
            laf->setAccentColour (accentColour);

        if (topLabel && label.isNotEmpty())
        {
            g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::textSecondary));
            g.setFont (juce::Font ("JetBrains Mono", getHeight() * 0.35f, juce::Font::bold));
            g.drawText (label.toUpperCase(), labelBounds, juce::Justification::centredLeft);
        }

        if (toggle.getToggleState())
        {
            g.setColour (accentColour.withAlpha (0.05f));
            g.fillRoundedRectangle (toggle.getBounds().toFloat(), 5.0f);
        }
    }

    juce::ToggleButton& getToggle() { return toggle; }

private:
    juce::ToggleButton toggle;
    juce::String label;
    juce::Colour accentColour;
    bool topLabel;
    juce::Rectangle<int> labelBounds;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleComponent)
};

/**
 * ComboBox with relative label.
 */
class DropdownComponent : public juce::Component
{
public:
    DropdownComponent (const juce::String& labelText,
                       juce::Colour accent = juce::Colour (CHORwerkLookAndFeel::Colors::voices))
        : label (labelText), accentColour (accent)
    {
        addAndMakeVisible (box);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, box);
    }

    void setTooltip (const juce::String& text) { box.setTooltip (text); }

    juce::ComboBox& getBox() { return box; }

    void resized() override
    {
        auto b = getLocalBounds();
        if (label.isNotEmpty())
        {
            labelBounds = b.removeFromTop (static_cast<int>(getHeight() * 0.45f));
            b.removeFromTop (2);
        }
        box.setBounds (b);
    }

    void paint (juce::Graphics& g) override
    {
        if (label.isNotEmpty())
        {
            g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::textSecondary));
            g.setFont (juce::Font ("JetBrains Mono", getHeight() * 0.35f, juce::Font::bold));
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
