#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/ChorusEngine.h"
#include "LookAndFeel.h"

/**
 * Real-time animated waveform display showing chorus voice delay modulation.
 * Each voice is drawn as a coloured oscillating line.
 */
class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    WaveformDisplay (ChorusEngine& engine)
        : chorusEngine (engine)
    {
        startTimerHz (30);  // 30 FPS animation
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour (juce::Colour (0xff070e17));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::border));
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        auto area = bounds.reduced (2.0f);
        float w = area.getWidth();
        float h = area.getHeight();
        float cx = area.getX();
        float cy = area.getCentreY();

        // Grid lines
        g.setColour (juce::Colour (0xff1a2535));
        for (float y = area.getY(); y < area.getBottom(); y += 20.0f)
        {
            g.drawHorizontalLine (static_cast<int> (y), area.getX(), area.getRight());
        }
        for (float x = area.getX(); x < area.getRight(); x += 40.0f)
        {
            g.drawVerticalLine (static_cast<int> (x), area.getY(), area.getBottom());
        }

        // Center line
        g.setColour (juce::Colour (0xff334155));
        float dashLengths[] = { 4.0f, 4.0f };
        g.drawDashedLine (juce::Line<float> (area.getX(), cy, area.getRight(), cy),
                          dashLengths, 2, 0.5f);

        // Voice colors
        const juce::Colour voiceColors[] = {
            juce::Colour (0xff6ee7b7), juce::Colour (0xff38bdf8),
            juce::Colour (0xffa78bfa), juce::Colour (0xfffb923c),
            juce::Colour (0xfff472b6), juce::Colour (0xfffacc15),
            juce::Colour (0xff34d399), juce::Colour (0xff818cf8)
        };

        int numVoices = chorusEngine.getActiveVoices();
        float time = animTime;

        for (int v = 0; v < numVoices; ++v)
        {
            float voiceDelay = chorusEngine.getVoiceDelay (v);
            float normDelay = voiceDelay / ChorusEngine::MaxDelayMs;

            juce::Path path;
            auto color = voiceColors[v % 8];

            for (int px = 0; px < static_cast<int> (w); ++px)
            {
                float xn = static_cast<float> (px) / w;
                // Animate: combine voice delay with time-based sine
                float y = cy + std::sin (xn * 6.0f + time * (0.8f + v * 0.15f)) *
                           (10.0f + normDelay * (h * 0.35f)) +
                           std::sin (xn * 12.0f + time * 1.7f + static_cast<float> (v)) *
                           5.0f * normDelay;

                y = juce::jlimit (area.getY() + 2.0f, area.getBottom() - 2.0f, y);

                if (px == 0)
                    path.startNewSubPath (cx + static_cast<float> (px), y);
                else
                    path.lineTo (cx + static_cast<float> (px), y);
            }

            g.setColour (color.withAlpha (0.7f));
            g.strokePath (path, juce::PathStrokeType (1.2f));
        }

        // Voice count label
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::textDim));
        g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
        g.drawText (juce::String (numVoices) + " voices",
                    area.reduced (6.0f), juce::Justification::topRight);
    }

private:
    void timerCallback() override
    {
        animTime += 0.04f;
        repaint();
    }

    ChorusEngine& chorusEngine;
    float animTime = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

/**
 * Small LFO waveform preview.
 */
class LFODisplay : public juce::Component,
                   private juce::Timer
{
public:
    LFODisplay (ChorusEngine& engine)
        : chorusEngine (engine)
    {
        startTimerHz (24);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff070e17));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::border));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        auto area = bounds.reduced (3.0f);
        float w = area.getWidth();
        float h = area.getHeight();
        float cy = area.getCentreY();

        // Center line
        g.setColour (juce::Colour (0xff1a2535));
        g.drawHorizontalLine (static_cast<int> (cy), area.getX(), area.getRight());

        // LFO waveform
        auto& lfo = chorusEngine.getLFO();
        juce::Path path;

        for (int px = 0; px < static_cast<int> (w); ++px)
        {
            float phase = static_cast<float> (px) / w;
            // We approximate by evaluating the LFO shape function
            // This is a visual preview - not the actual LFO output
            float val = previewLFOShape (phase);
            float y = cy - val * (h * 0.45f);

            if (px == 0)
                path.startNewSubPath (area.getX() + static_cast<float> (px), y);
            else
                path.lineTo (area.getX() + static_cast<float> (px), y);
        }

        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::lfo));
        g.strokePath (path, juce::PathStrokeType (1.5f));

        // Phase indicator (vertical line at current LFO phase)
        float phaseX = area.getX() + lfo.getPhase() * w;
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::lfo).withAlpha (0.4f));
        g.drawVerticalLine (static_cast<int> (phaseX), area.getY(), area.getBottom());
    }

    void setShape (int shapeIdx) { currentShape = shapeIdx; }

private:
    void timerCallback() override { repaint(); }

    float previewLFOShape (float phase) const
    {
        float p = phase;
        switch (currentShape)
        {
            case 0: return std::sin (p * juce::MathConstants<float>::twoPi);            // Sine
            case 1: return 2.0f * p - 1.0f;                                              // Ramp
            case 2: return (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);         // Triangle
            case 3: return (p < 0.5f) ? 1.0f : -1.0f;                                   // Square
            case 4: return (std::sin (p * 13.7f) > 0) ? 0.8f : -0.8f;                   // Stepping random (approx)
            case 5: return std::sin (p * 3.0f) * std::cos (p * 7.3f);                   // Smooth random (approx)
            case 6: return std::sin (p * juce::MathConstants<float>::twoPi);             // User (show sine as fallback)
            default: return 0.0f;
        }
    }

    ChorusEngine& chorusEngine;
    int currentShape = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFODisplay)
};

/**
 * Envelope level meter.
 */
class EnvelopeMeter : public juce::Component,
                      private juce::Timer
{
public:
    EnvelopeMeter (ChorusEngine& engine)
        : chorusEngine (engine)
    {
        startTimerHz (20);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff070e17));
        g.fillRoundedRectangle (bounds, 3.0f);

        float level = chorusEngine.getEnvelopeLevel();
        smoothedLevel += (level - smoothedLevel) * 0.3f;

        auto fillBounds = bounds;
        fillBounds.setHeight (bounds.getHeight() * smoothedLevel);
        fillBounds.setY (bounds.getBottom() - fillBounds.getHeight());

        auto color = juce::Colour (CharsiesiLookAndFeel::Colors::envelope);
        g.setGradientFill (juce::ColourGradient (
            color.withAlpha (0.3f), 0, fillBounds.getBottom(),
            color.withAlpha (0.8f), 0, fillBounds.getY(), false));
        g.fillRoundedRectangle (fillBounds, 3.0f);

        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::border));
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    }

private:
    void timerCallback() override { repaint(); }
    ChorusEngine& chorusEngine;
    float smoothedLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeMeter)
};
