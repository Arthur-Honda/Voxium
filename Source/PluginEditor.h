#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VoxiumLookAndFeel.h"


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

	VoxiumLookAndFeel voxiumLookAndFeel;

	juce::Label titleLabel;      // "VOXIUM"
	juce::Label noteLabel;       // nota detectada, grande (ex: "E4")
	juce::Label frequencyLabel;  // Hz, pequeno, abaixo da nota
	juce::Label scaleStatusLabel; // "IN SCALE" / "OUT OF SCALE" / "--"
	juce::Label harmonyLabel;    // "-> G4"

	juce::Label keyFieldLabel;
	juce::Label scaleFieldLabel;
	juce::Label harmonyFieldLabel;

	juce::ComboBox keyComboBox;
	juce::ComboBox scaleComboBox;
	juce::ComboBox harmonyComboBox;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessorEditor)
};