#include "PluginProcessor.h"
#include "PluginEditor.h"

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(400, 300);

	pitchLabel.setText("Aguardando audio...", juce::dontSendNotification);
	pitchLabel.setJustificationType(juce::Justification::centred);
	pitchLabel.setFont(juce::Font(24.0f));
	addAndMakeVisible(pitchLabel);

	startTimerHz(15); // atualiza o texto 15 vezes por segundo
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor() {}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void VoxiumAudioProcessorEditor::resized()
{
	pitchLabel.setBounds(getLocalBounds());
}

// USED FOR DEBUGGING PURPOSES
//void VoxiumAudioProcessorEditor::timerCallback()
//{
//	float freq = audioProcessor.getCurrentPitch();
//	float level = audioProcessor.getCurrentLevel();
//
//	pitchLabel.setText("Pitch: " + juce::String(freq, 1) + " Hz | Level: " + juce::String(level, 5), juce::dontSendNotification);
//}


void VoxiumAudioProcessorEditor::timerCallback() {
	float freq = audioProcessor.getCurrentPitch();

	if (freq > 0.0f) {
		NoteInfo note = audioProcessor.getCurrentNote();
		pitchLabel.setText(juce::String(freq, 1) + " Hz  -  " + note.name, juce::dontSendNotification);
	}
	else {
		pitchLabel.setText("Aguardando audio...", juce::dontSendNotification);
	}
}