#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VoxiumAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	VoxiumAudioProcessorEditor(VoxiumAudioProcessor&);
	~VoxiumAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	VoxiumAudioProcessor& audioProcessor;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessorEditor)
};