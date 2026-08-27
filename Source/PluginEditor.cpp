#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setLookAndFeel(&voxiumLookAndFeel);
	setSize(420, 460);

	// --- Titulo ---
	titleLabel.setText("VOXIUM", juce::dontSendNotification);
	titleLabel.setFont(juce::Font(juce::FontOptions(15.0f)).withExtraKerningFactor(0.15f));
	titleLabel.setColour(juce::Label::textColourId, VoxiumColours::textSecondary);
	titleLabel.setJustificationType(juce::Justification::centredLeft);
	addAndMakeVisible(titleLabel);

	// --- Readout principal: nota detectada ---
	noteLabel.setText("--", juce::dontSendNotification);
	noteLabel.setFont(juce::Font(juce::FontOptions(56.0f)));
	noteLabel.setColour(juce::Label::textColourId, VoxiumColours::textPrimary);
	noteLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(noteLabel);

	frequencyLabel.setText("Waiting for audio...", juce::dontSendNotification);
	frequencyLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
	frequencyLabel.setColour(juce::Label::textColourId, VoxiumColours::textSecondary);
	frequencyLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(frequencyLabel);

	scaleStatusLabel.setText("", juce::dontSendNotification);
	scaleStatusLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
	scaleStatusLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(scaleStatusLabel);

	harmonyLabel.setText("", juce::dontSendNotification);
	harmonyLabel.setFont(juce::Font(juce::FontOptions(16.0f)));
	harmonyLabel.setColour(juce::Label::textColourId, VoxiumColours::accent);
	harmonyLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(harmonyLabel);

	// --- Legendas dos seletores ---
	auto setupFieldLabel = [this](juce::Label& label, const juce::String& text)
		{
			label.setText(text, juce::dontSendNotification);
			label.setFont(juce::Font(juce::FontOptions(11.0f)).withExtraKerningFactor(0.1f));
			label.setColour(juce::Label::textColourId, VoxiumColours::textSecondary);
			label.setJustificationType(juce::Justification::centredLeft);
			addAndMakeVisible(label);
		};

	setupFieldLabel(keyFieldLabel, "KEY");
	setupFieldLabel(scaleFieldLabel, "SCALE");
	setupFieldLabel(harmonyFieldLabel, "HARMONY");

	// --- ComboBoxes (mesma logica de antes, so o visual muda via LookAndFeel) ---
	static const juce::StringArray keyNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	for (int i = 0; i < keyNames.size(); ++i)
		keyComboBox.addItem(keyNames[i], i + 1);

	keyComboBox.setSelectedId(1, juce::dontSendNotification);
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

	scaleComboBox.setSelectedId(1, juce::dontSendNotification);
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

	harmonyComboBox.setSelectedId(3, juce::dontSendNotification);
	harmonyComboBox.onChange = [this]
		{
			static const std::vector<int> offsets = { -2, 0, 2, 4, 7 };
			int selectedIndex = harmonyComboBox.getSelectedId() - 1;
			audioProcessor.setHarmonyDegreeOffset(offsets[(size_t)selectedIndex]);
		};
	addAndMakeVisible(harmonyComboBox);

	startTimerHz(15);
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(VoxiumColours::background);

	// pontinho accent do lado do titulo, um toque de identidade visual
	auto titleBounds = titleLabel.getBounds();
	g.setColour(VoxiumColours::accent);
	g.fillEllipse((float)titleBounds.getRight() + 8.0f, (float)titleBounds.getCentreY() - 3.0f, 6.0f, 6.0f);

	// linha divisoria sutil entre o readout e os seletores
	g.setColour(VoxiumColours::border);
	int dividerY = harmonyLabel.getBottom() + 20;
	g.drawHorizontalLine(dividerY, 24.0f, (float)getWidth() - 24.0f);
}

void VoxiumAudioProcessorEditor::resized()
{
	auto area = getLocalBounds().reduced(24, 20);

	titleLabel.setBounds(area.removeFromTop(20));

	area.removeFromTop(20);

	noteLabel.setBounds(area.removeFromTop(64));
	frequencyLabel.setBounds(area.removeFromTop(20));
	scaleStatusLabel.setBounds(area.removeFromTop(20));

	area.removeFromTop(4);
	harmonyLabel.setBounds(area.removeFromTop(24));

	area.removeFromTop(28); // espaco pra linha divisoria desenhada no paint()

	auto layoutField = [&area](juce::Label& fieldLabel, juce::ComboBox& box)
		{
			auto row = area.removeFromTop(56);
			fieldLabel.setBounds(row.removeFromTop(16));
			row.removeFromTop(4);
			box.setBounds(row.removeFromTop(32));
		};

	layoutField(keyFieldLabel, keyComboBox);
	area.removeFromTop(12);
	layoutField(scaleFieldLabel, scaleComboBox);
	area.removeFromTop(12);
	layoutField(harmonyFieldLabel, harmonyComboBox);
}

void VoxiumAudioProcessorEditor::timerCallback()
{
	float freq = audioProcessor.getCurrentPitch();

	if (freq > 0.0f)
	{
		NoteInfo note = audioProcessor.getCurrentNote();
		NoteInfo harmony = audioProcessor.getHarmonyNote();
		bool inScale = audioProcessor.isCurrentNoteInScale();

		noteLabel.setText(note.name, juce::dontSendNotification);
		frequencyLabel.setText(juce::String(freq, 1) + " Hz", juce::dontSendNotification);

		scaleStatusLabel.setText(inScale ? "IN SCALE" : "OUT OF SCALE", juce::dontSendNotification);
		scaleStatusLabel.setColour(juce::Label::textColourId,
			inScale ? VoxiumColours::accent : VoxiumColours::textSecondary);

		harmonyLabel.setText(juce::String(juce::CharPointer_UTF8("\xe2\x86\x92 ")) + harmony.name, juce::dontSendNotification);
	}
	else
	{
		noteLabel.setText("--", juce::dontSendNotification);
		frequencyLabel.setText("Waiting for audio...", juce::dontSendNotification);
		scaleStatusLabel.setText("", juce::dontSendNotification);
		harmonyLabel.setText("", juce::dontSendNotification);
	}
}