#pragma once

#include <JuceHeader.h>
#include "DSP/PitchDetector.h"
#include "DSP/NoteUtils.h"

class VoxiumAudioProcessor : public juce::AudioProcessor // classe filho : classe pai
{
public:
	VoxiumAudioProcessor();
	~VoxiumAudioProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override; // pega informações do áudio como sample rate e samples per block
	void releaseResources() override; // quando o DAW não necessita mais do plugin

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override; // aceita canal de áudio mono/stereo

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; // O DAW manda blocos de áudio para o plugin, e o 
																			 // processBlock() é chamado para você processar esses blocos.

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

	float getCurrentPitch() const { return currentPitch.load(); }
	float getCurrentLevel() const { return currentLevel.load(); } // DEBUGGING Purposes
	NoteInfo getCurrentNote() const {return NoteUtils::frequencyToNote(currentPitch.load()); }

private:
	juce::AudioProcessorValueTreeState parameters;

	PitchDetector pitchDetector;
	std::atomic<float> currentPitch{ 0.0f }; // variável atômica para armazenar a frequência detectada
	std::atomic<float> currentLevel{ 0.0f }; // tempo

	static constexpr int pitchAnalysisSize = 2048; // janela de análise, independente do tamanho do bloco do driver
	std::vector<float> pitchAnalysisBuffer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessor)
};