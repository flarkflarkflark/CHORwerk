#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorEditor* CharsiesiProcessor::createEditor()
{
    return new CharsiesiEditor (*this);
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CharsiesiProcessor();
}
