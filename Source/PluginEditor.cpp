#include "PluginProcessor.h"
#include "PluginEditor.h"

VoxiumAudioProcessorEditor::VoxiumAudioProcessorEditor(VoxiumAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(400, 300);
}

VoxiumAudioProcessorEditor::~VoxiumAudioProcessorEditor() {}

void VoxiumAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void VoxiumAudioProcessorEditor::resized() {}