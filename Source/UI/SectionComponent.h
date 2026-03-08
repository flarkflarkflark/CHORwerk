#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

/**
 * Visual section wrapper that draws a bordered panel with a floating title badge.
 * Children are placed in the content area (below the title).
 */
class SectionComponent : public juce::Component
{
public:
    SectionComponent (const juce::String& title, juce::Colour accent)
        : sectionTitle (title), accentColour (accent) {}

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        // Panel background
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::surface));
        g.fillRoundedRectangle (b, 8.0f);

        // Border
        g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::border).withAlpha (0.6f));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);

        // Title badge
        if (sectionTitle.isNotEmpty())
        {
            g.setFont (juce::Font ("JetBrains Mono", 10.0f, juce::Font::bold));
            auto textWidth = g.getCurrentFont().getStringWidth (sectionTitle.toUpperCase()) + 12;

            auto badgeBounds = juce::Rectangle<float> (b.getX() + 12.0f, b.getY() - 7.0f,
                                                        static_cast<float> (textWidth), 14.0f);

            // Badge background (covers border)
            g.setColour (juce::Colour (CharsiesiLookAndFeel::Colors::surface));
            g.fillRect (badgeBounds);

            // Badge text
            g.setColour (accentColour);
            g.drawText (sectionTitle.toUpperCase(), badgeBounds, juce::Justification::centred);
        }
    }

    /** Returns the content area (inset from borders, below title). */
    juce::Rectangle<int> getContentBounds() const
    {
        return getLocalBounds().reduced (10, 4).withTrimmedTop (10);
    }

private:
    juce::String sectionTitle;
    juce::Colour accentColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};
