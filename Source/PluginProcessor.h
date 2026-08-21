#pragma once

#include <cmath>
#include <JuceHeader.h>
#include "DSP/PitchDetector.h"
#include "DSP/NoteUtils.h"
#include "DSP/ScaleUtils.h"
#include "DSP/HarmonyUtils.h"

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

	void setSelectedKey(int newKeyRootNote) { selectedKeyRootNote = newKeyRootNote; }
	void setSelectedScale(ScaleType newScaleType) { selectedScaleType = newScaleType; }

	bool isCurrentNoteInScale() const
	{
		NoteInfo note = getCurrentNote();
		if (note.midiNoteNumber < 0)
			return false;

		int noteClass = ((note.midiNoteNumber % 12) + 12) % 12;
		return ScaleUtils::isNoteInScale(noteClass, selectedKeyRootNote, selectedScaleType);
	}

	NoteInfo getHarmonyNote() const
	{
		NoteInfo original = getCurrentNote();
		if (original.midiNoteNumber < 0)
			return { "", -1, 0.0f };

		int harmonyMidi = HarmonyUtils::getHarmonyNote(original.midiNoteNumber, selectedKeyRootNote, selectedScaleType, 2); // 2 = terca acima

		// reaproveita o NoteUtils pra montar o nome da nota harmonica a partir do numero MIDI
		float harmonyFrequency = 440.0f * std::pow(2.0f, (harmonyMidi - 69) / 12.0f);
		return NoteUtils::frequencyToNote(harmonyFrequency);
	}

private:
	juce::AudioProcessorValueTreeState parameters;

	PitchDetector pitchDetector;
	std::atomic<float> currentPitch{ 0.0f }; // variável atômica para armazenar a frequência detectada
	std::atomic<float> currentLevel{ 0.0f }; // tempo

	static constexpr int pitchAnalysisSize = 2048; // janela de análise, independente do tamanho do bloco do driver
	std::vector<float> pitchAnalysisBuffer;

	int selectedKeyRootNote = 0; // 0 = C, por padrao
	ScaleType selectedScaleType = ScaleType::Major; // Major, por padrao

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessor)
};