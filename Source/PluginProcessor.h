#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "DSP/ChorusEngine.h"
#include "Presets/PresetManager.h"

class CHORwerkProcessor : public juce::AudioProcessor
{
public:
    CHORwerkProcessor()
        : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
          apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
    {
    }

    ~CHORwerkProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        chorusEngine.prepare (sampleRate, samplesPerBlock);

        // Initialize presets on first prepare (safe for audio thread)
        if (! presetsInitialized)
        {
            presetManager.initialize();
            presetsInitialized = true;
        }
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;

        return true;
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numSamples <= 0 || numChannels <= 0)
            return;
            
        chorusEngine.process (buffer, getPlayHead(), apvts);
        
        // Ensure any additional channels are cleared (e.g. if host gives 4 channels for a stereo plugin)
        for (int i = 2; i < numChannels; ++i)
            buffer.clear (i, 0, numSamples);
    }

    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "CHORwerk v1.0.0"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }  // ~50ms max delay

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override
    {
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
        if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }

    juce::AudioProcessorValueTreeState apvts;
    ChorusEngine& getChorusEngine() { return chorusEngine; }
    PresetManager& getPresetManager() { return presetManager; }

private:
    ChorusEngine chorusEngine;
    PresetManager presetManager { apvts };
    bool presetsInitialized = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CHORwerkProcessor)
};
