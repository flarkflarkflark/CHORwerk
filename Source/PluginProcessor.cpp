#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorEditor* CHORwerkProcessor::createEditor()
{
    return new CHORwerkEditor (*this);
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CHORwerkProcessor();
}
