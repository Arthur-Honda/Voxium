#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p) {
	setLookAndFeel(&voxiumLookAndFeel);
	setSize(420, 540);

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
	auto setupFieldLabel = [this](juce::Label& label, const juce::String& text) {
			label.setText(text, juce::dontSendNotification);
			label.setFont(juce::Font(juce::FontOptions(11.0f)).withExtraKerningFactor(0.1f));
			label.setColour(juce::Label::textColourId, VoxiumColours::textSecondary);
			label.setJustificationType(juce::Justification::centredLeft);
			addAndMakeVisible(label);
		};

	setupFieldLabel(keyFieldLabel, "KEY");
	setupFieldLabel(scaleFieldLabel, "SCALE");
	setupFieldLabel(harmonyFieldLabel, "HARMONY");
	setupFieldLabel(mixFieldLabel, "MIX");
	mixFieldLabel.setJustificationType(juce::Justification::centred);

	// --- ComboBoxes: os itens continuam precisando bater, na mesma ordem,
	// com as "choices" dos parametros no APVTS (ver createParameterLayout
	// no PluginProcessor.cpp) -- mas quem sincroniza selecao <-> parametro
	// agora e o ComboBoxAttachment, nao mais um onChange manual.
	static const juce::StringArray keyNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	for (int i = 0; i < keyNames.size(); ++i)
		keyComboBox.addItem(keyNames[i], i + 1);
	addAndMakeVisible(keyComboBox);
	keyAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.getAPVTS(), "key", keyComboBox);

	static const juce::StringArray scaleNames{ "Major", "Natural Minor", "Harmonic Minor", "Melodic Minor" };
	for (int i = 0; i < scaleNames.size(); ++i)
		scaleComboBox.addItem(scaleNames[i], i + 1);
	addAndMakeVisible(scaleComboBox);
	scaleAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.getAPVTS(), "scale", scaleComboBox);

	static const juce::StringArray harmonyNames{ "3rd below", "Unison", "3rd above", "5th above", "Octave" };
	for (int i = 0; i < harmonyNames.size(); ++i)
		harmonyComboBox.addItem(harmonyNames[i], i + 1);
	addAndMakeVisible(harmonyComboBox);
	harmonyAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.getAPVTS(), "harmony", harmonyComboBox);

	mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
	mixSlider.setColour(juce::Slider::textBoxTextColourId, VoxiumColours::textPrimary);
	mixSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
	mixSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	addAndMakeVisible(mixSlider);
	mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "mix", mixSlider);


	startTimerHz(15);
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor() {
	setLookAndFeel(nullptr);
}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g) {
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

void VoxiumAudioProcessorEditor::resized() {
	auto area = getLocalBounds().reduced(24, 20);

	titleLabel.setBounds(area.removeFromTop(20));

	area.removeFromTop(20);

	noteLabel.setBounds(area.removeFromTop(64));
	frequencyLabel.setBounds(area.removeFromTop(20));
	scaleStatusLabel.setBounds(area.removeFromTop(20));

	area.removeFromTop(4);
	harmonyLabel.setBounds(area.removeFromTop(24));

	area.removeFromTop(28); // espaco pra linha divisoria desenhada no paint()

	auto layoutField = [&area](juce::Label& fieldLabel, juce::ComboBox& box) {
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

	area.removeFromTop(20);
	mixFieldLabel.setBounds(area.removeFromTop(16));
	area.removeFromTop(4);
	auto sliderArea = area.removeFromTop(90);
	mixSlider.setBounds(sliderArea.withSizeKeepingCentre(90, 90));
}

void VoxiumAudioProcessorEditor::timerCallback() {
	float freq = audioProcessor.getCurrentPitch();

	if (freq > 0.0f) {
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
	else {
		noteLabel.setText("--", juce::dontSendNotification);
		frequencyLabel.setText("Waiting for audio...", juce::dontSendNotification);
		scaleStatusLabel.setText("", juce::dontSendNotification);
		harmonyLabel.setText("", juce::dontSendNotification);
	}
}