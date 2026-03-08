#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/ChorusEngine.h"
#include "LookAndFeel.h"

/**
 * 8-step sequencer with clickable/draggable bars.
 * Each bar represents a step value (0–100%).
 * Highlights the currently playing step.
 */
class StepSequencerComponent : public juce::Component,
                               private juce::Timer
{
public:
    static constexpr int NumSteps = 8;

    StepSequencerComponent (ChorusEngine& engine)
        : chorusEngine (engine)
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            sliders[i].setSliderStyle (juce::Slider::LinearBarVertical);
            sliders[i].setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
            sliders[i].setRange (0.0, 100.0, 0.1);
            sliders[i].setValue (50.0, juce::dontSendNotification);
            addAndMakeVisible (sliders[i]);
        }

        startTimerHz (15);
    }

    void attachToAPVTS (juce::AudioProcessorValueTreeState& apvts)
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            auto paramId = "seq" + juce::String (i);
            attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                apvts, paramId, sliders[i]);
        }
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (0, 2);
        float stepW = static_cast<float> (b.getWidth()) / static_cast<float> (NumSteps);

        for (int i = 0; i < NumSteps; ++i)
        {
            int x = b.getX() + static_cast<int> (static_cast<float> (i) * stepW);
            int w = static_cast<int> (stepW) - 2;
            sliders[i].setBounds (x, b.getY(), w, b.getHeight());
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        float stepW = b.getWidth() / static_cast<float> (NumSteps);
        auto color = juce::Colour (CharsiesiLookAndFeel::Colors::stepSeq);

        int currentStep = chorusEngine.getSequencer().getCurrentStep();

        for (int i = 0; i < NumSteps; ++i)
        {
            float x = b.getX() + static_cast<float> (i) * stepW;
            auto stepBounds = juce::Rectangle<float> (x, b.getY(), stepW - 2.0f, b.getHeight());

            // Background
            g.setColour (juce::Colour (0xff0a1018));
            g.fillRoundedRectangle (stepBounds, 2.0f);

            // Fill
            float value = static_cast<float> (sliders[i].getValue()) / 100.0f;
            auto fillBounds = stepBounds;
            fillBounds.setHeight (stepBounds.getHeight() * value);
            fillBounds.setY (stepBounds.getBottom() - fillBounds.getHeight());

            float alpha = (i == currentStep) ? 0.9f : 0.5f;
            g.setGradientFill (juce::ColourGradient (
                color.withAlpha (alpha * 0.3f), 0, fillBounds.getBottom(),
                color.withAlpha (alpha), 0, fillBounds.getY(), false));
            g.fillRoundedRectangle (fillBounds, 2.0f);

            // Border
            g.setColour (i == currentStep
                         ? color.withAlpha (0.6f)
                         : juce::Colour (0xff1a2535));
            g.drawRoundedRectangle (stepBounds, 2.0f, 1.0f);
        }
    }

    void paintOverChildren (juce::Graphics&) override
    {
        // Sliders are transparent — paint handles the visuals
    }

private:
    void timerCallback() override { repaint(); }

    ChorusEngine& chorusEngine;
    juce::Slider sliders[NumSteps];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachments[NumSteps];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepSequencerComponent)
};
