#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/PresetBrowser.h"
#include "UI/KnobComponent.h"
#include "UI/WaveformDisplay.h"
#include "UI/StepSequencerComponent.h"
#include "UI/SectionComponent.h"

class CharsiesiEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit CharsiesiEditor (CharsiesiProcessor& p);
    ~CharsiesiEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutSection (juce::Rectangle<int>& area, SectionComponent& section,
                        std::initializer_list<juce::Component*> children,
                        int rows, int cols);

    CharsiesiProcessor& processor;
    CharsiesiLookAndFeel lookAndFeel;

    static constexpr int baseWidth = 760;
    static constexpr int baseHeight = 520;

    //==========================================================================
    // Header
    //==========================================================================
    PresetBrowser presetBrowser;
    ToggleComponent preTgl, postTgl;

    //==========================================================================
    // TOP ROW: Waveform + Voices
    //==========================================================================
    WaveformDisplay waveformDisplay;
    SectionComponent voicesSection;
    KnobComponent voicesKnob, mixKnob, feedbackKnob, rotationKnob;
    ToggleComponent stereoTgl, followLevelTgl, collisionTgl;

    //==========================================================================
    // MIDDLE ROW: Rate, Delay, Filter
    //==========================================================================
    SectionComponent rateSection, delaySection, filterSection;
    KnobComponent minRateKnob, rateRangeKnob, rateUpdateKnob;
    ToggleComponent quantizeTgl;
    KnobComponent minDelayKnob, delayRangeKnob;
    KnobComponent lowpassKnob, lpResKnob, highpassKnob, hpResKnob;

    //==========================================================================
    // BOTTOM ROW: Step Seq, Envelope, LFO
    //==========================================================================
    SectionComponent seqSection, envSection, lfoSection;

    // Step Sequencer
    StepSequencerComponent stepSeqDisplay;
    KnobComponent seqStepKnob;
    DropdownComponent seqUnitDrop;
    ToggleComponent seqQuantTgl;

    // Envelope
    KnobComponent envLPKnob, envHPKnob, envDelayKnob;
    DropdownComponent followModeDrop;
    EnvelopeMeter envMeter;

    // LFO
    KnobComponent lfoRateKnob;
    DropdownComponent lfoShapeDrop, lfoUnitDrop;
    ToggleComponent lfoQuantTgl;
    LFODisplay lfoDisplay;

    //==========================================================================
    // MOD DEPTH: small knobs for Seq/Env/LFO → LP/HP/Delay
    //==========================================================================
    KnobComponent seqLPKnob, seqHPKnob, seqDelayKnob;
    KnobComponent lfoLPKnob, lfoHPKnob, lfoDelayKnob;

    // Stereo / Update controls
    DropdownComponent stereoModeDrop, updateUnitDrop;
    ToggleComponent updateQuantTgl;

    // Gain controls
    KnobComponent pregainKnob, postgainKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CharsiesiEditor)
};
