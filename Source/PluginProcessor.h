#pragma once

#include <JuceHeader.h>

class VoxiumAudioProcessor : public juce::AudioProcessor // classe filho : classe pai
{
public:
	VoxiumAudioProcessor();
	~VoxiumAudioProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override; // pega informações do áudio como sample rate e samples per block
	void releaseResources() override; // quando o DAW não necessita mais do plugin

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override; // aceita canal de áudio mono/stereo

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; // processa o áudio, manda blocos de áudio para o plugin

	juce::AudioProcessorEditor* createEditor() override; // cria a interface gráfica do plugin
	bool hasEditor() const override; // possui interface gráfica?

	const juce::String getName() const override; // retorna o nome do plugin

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override; // não será usado midi. Voxium é um plugin de ÁUDIO.
	double getTailLengthSeconds() const override;

	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override; // serve para o estado do plugin.

private:
	juce::AudioProcessorValueTreeState parameters;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessor)
};