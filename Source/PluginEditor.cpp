#include "PluginProcessor.h"
#include "PluginEditor.h"

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(400, 350);

	pitchLabel.setText("Waiting for audio...", juce::dontSendNotification);
	pitchLabel.setJustificationType(juce::Justification::centred);
	pitchLabel.setFont(juce::Font(24.0f));
	addAndMakeVisible(pitchLabel);

	static const juce::StringArray keyNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	for (int i = 0; i < keyNames.size(); ++i)
		keyComboBox.addItem(keyNames[i], i + 1); // JUCE ComboBox IDs comecam em 1, nao em 0

	keyComboBox.setSelectedId(1, juce::dontSendNotification); // C por padrao
	keyComboBox.onChange = [this]
		{
			audioProcessor.setSelectedKey(keyComboBox.getSelectedId() - 1);
		};
	addAndMakeVisible(keyComboBox);

	static const juce::StringArray scaleNames{ "Major", "Natural Minor", "Harmonic Minor", "Melodic Minor",
												 "Major Pentatonic", "Minor Pentatonic", "Dorian", "Phrygian",
												 "Lydian", "Mixolydian", "Locrian" };
	for (int i = 0; i < scaleNames.size(); ++i)
		scaleComboBox.addItem(scaleNames[i], i + 1);

	scaleComboBox.setSelectedId(1, juce::dontSendNotification); // Major por padrao
	scaleComboBox.onChange = [this]
		{
			audioProcessor.setSelectedScale(static_cast<ScaleType>(scaleComboBox.getSelectedId() - 1));
		};
	addAndMakeVisible(scaleComboBox);

	startTimerHz(15); // atualiza o texto 15 vezes por segundo
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor() {}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void VoxiumAudioProcessorEditor::resized()
{
	auto area = getLocalBounds();
	pitchLabel.setBounds(area.removeFromTop(200));
	keyComboBox.setBounds(area.removeFromTop(40).reduced(20, 5));
	scaleComboBox.setBounds(area.removeFromTop(40).reduced(20, 5));
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
		NoteInfo harmony = audioProcessor.getHarmonyNote();
		bool inScale = audioProcessor.isCurrentNoteInScale();

		juce::String text = juce::String(freq, 1) + " Hz  -  " + note.name;
		text += inScale ? "  (In scale)" : "  (Out of scale)";
		text += "\nHarmony (3rd): " + juce::String(harmony.name);

		pitchLabel.setText(text, juce::dontSendNotification);
	}
	else {
		pitchLabel.setText("Waiting for audio...", juce::dontSendNotification);
	}
}