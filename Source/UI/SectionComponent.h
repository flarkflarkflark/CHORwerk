#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

/**
 * Visual section wrapper with consistent left-aligned tab badges.
 */
class SectionComponent : public juce::Component
{
public:
    SectionComponent (const juce::String& title, juce::Colour accent)
        : sectionTitle (title), accentColour (accent) {}

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        float h = b.getHeight();
        float cornerSize = 6.0f;
        
        // Consistent header height based on global scaling
        float headerH = 22.0f * currentScale;
        if (headerH < 16.0f) headerH = 16.0f;

        // Panel background
        g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::surface));
        g.fillRoundedRectangle (b, cornerSize);

        // Header Badge (Tab)
        if (sectionTitle.isNotEmpty())
        {
            g.setFont (juce::Font ("JetBrains Mono", headerH * 0.55f, juce::Font::bold));
            float textW = g.getCurrentFont().getStringWidth (sectionTitle.toUpperCase());
            float tabW = textW + 20.0f * currentScale;
            
            auto headerArea = b.removeFromTop (headerH).removeFromLeft (tabW);
            
            // Tab background gradient
            g.setGradientFill (juce::ColourGradient (
                accentColour.withAlpha (0.3f), 0, headerArea.getY(),
                accentColour.withAlpha (0.05f), 0, headerArea.getBottom(), false));
            g.fillRoundedRectangle (headerArea, cornerSize);
            g.fillRect (headerArea.withTrimmedTop (headerH / 2.0f));

            // Accent line
            g.setColour (accentColour);
            g.fillRect (headerArea.removeFromTop (2.0f * currentScale));

            // Title text
            g.setColour (accentColour);
            g.drawText (sectionTitle.toUpperCase(), headerArea, juce::Justification::centred);
        }

        // Border
        g.setColour (juce::Colour (CHORwerkLookAndFeel::Colors::border).withAlpha (0.6f));
        g.drawRoundedRectangle (getLocalBounds().toFloat(), cornerSize, 1.0f);
    }

    juce::Rectangle<int> getContentBounds() const
    {
        float headerH = 22.0f * currentScale;
        if (headerH < 16.0f) headerH = 16.0f;
        return getLocalBounds().reduced (10, 8).withTrimmedTop (static_cast<int>(headerH));
    }

    void setScale (float s) { currentScale = s; repaint(); }

private:
    juce::String sectionTitle;
    juce::Colour accentColour;
    float currentScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};
