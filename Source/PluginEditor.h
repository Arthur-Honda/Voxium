#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class VoxiumAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
	VoxiumAudioProcessorEditor(VoxiumAudioProcessor&);
	~VoxiumAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	void timerCallback() override;

	VoxiumAudioProcessor& audioProcessor;
	juce::Label pitchLabel;
	juce::ComboBox keyComboBox;
	juce::ComboBox scaleComboBox;
	juce::ComboBox harmonyComboBox;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessorEditor)
};