#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(400, 400);

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

	static const std::vector<std::pair<juce::String, int>> harmonyOptions = {
		{ "3rd below", -2 },
		{ "Unison",     0 },
		{ "3rd above",  2 },
		{ "5th above",  4 },
		{ "Octave",     7 }
	};

	for (int i = 0; i < (int)harmonyOptions.size(); ++i)
		harmonyComboBox.addItem(harmonyOptions[(size_t)i].first, i + 1);

	harmonyComboBox.setSelectedId(3, juce::dontSendNotification); // "3rd above" por padrao
	harmonyComboBox.onChange = [this]
		{
			static const std::vector<int> offsets = { -2, 0, 2, 4, 7 };
			int selectedIndex = harmonyComboBox.getSelectedId() - 1;
			audioProcessor.setHarmonyDegreeOffset(offsets[(size_t)selectedIndex]);
		};
	addAndMakeVisible(harmonyComboBox);

	startTimerHz(15); // atualiza o texto 15 vezes por segundo
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor() {}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void VoxiumAudioProcessorEditor::resized() {
	auto area = getLocalBounds();
	pitchLabel.setBounds(area.removeFromTop(150));
	keyComboBox.setBounds(area.removeFromTop(40).reduced(20, 5));
	scaleComboBox.setBounds(area.removeFromTop(40).reduced(20, 5));
	harmonyComboBox.setBounds(area.removeFromTop(40).reduced(20, 5));
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
		text += "\nHarmony: " + juce::String(harmony.name);

		pitchLabel.setText(text, juce::dontSendNotification);
	}
	else {
		pitchLabel.setText("Waiting for audio...", juce::dontSendNotification);
	}
}