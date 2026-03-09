#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/PresetBrowser.h"
#include "UI/KnobComponent.h"
#include "UI/WaveformDisplay.h"
#include "UI/StepSequencerComponent.h"
#include "UI/SectionComponent.h"

class CHORwerkEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit CHORwerkEditor (CHORwerkProcessor& p);
    ~CHORwerkEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setScale (float newScale);

private:
    void timerCallback() override;

    CHORwerkProcessor& processor;
    CHORwerkLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltipWindow { this };

    float currentScale = 1.0f;

    static constexpr int baseWidth = 860;
    static constexpr int baseHeight = 640;

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
    SectionComponent rateSection;
    KnobComponent minRateKnob, rateRangeKnob, rateUpdateKnob;
    ToggleComponent quantizeTgl;
    SectionComponent delaySection;
    KnobComponent minDelayKnob, delayRangeKnob;
    SectionComponent filterSection;
    KnobComponent lowpassKnob, lpResKnob, highpassKnob, hpResKnob;

    //==========================================================================
    // BOTTOM ROW: Step Seq, Envelope, LFO
    //==========================================================================
    SectionComponent seqSection;

    // Step Sequencer
    StepSequencerComponent stepSeqDisplay;
    KnobComponent seqStepKnob;
    DropdownComponent seqUnitDrop;
    ToggleComponent seqQuantTgl;

    // Envelope
    SectionComponent envSection;
    KnobComponent envLPKnob, envHPKnob, envDelayKnob;
    DropdownComponent followModeDrop;
    EnvelopeMeter envMeter;

    // LFO
    SectionComponent lfoSection;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CHORwerkEditor)
};
