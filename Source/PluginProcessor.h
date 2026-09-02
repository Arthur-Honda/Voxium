#pragma once

#include <cmath>
#include <JuceHeader.h>
#include "DSP/PitchDetector.h"
#include "DSP/PitchShifter.h"
#include "Theory/NoteUtils.h"
#include "Theory/ScaleUtils.h"
#include "Theory/HarmonyUtils.h"

class VoxiumAudioProcessor : public juce::AudioProcessor {  // classe filho : classe pai
public:
	VoxiumAudioProcessor();
	~VoxiumAudioProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override; // pega informações do áudio como sample rate e samples per block
	void releaseResources() override; // pega informações do áudio como sample rate e samples per block

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override; // aceita canal de áudio mono/stereo

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; // O DAW manda blocos de áudio para o plugin, e o 
	// processBlock() é chamado para você processar esses blocos.

	juce::AudioProcessorEditor* createEditor() override; // cria a interface gráfica do plugin
	bool hasEditor() const override; // possui interface gráfica?

	const juce::String getName() const override;  // retorna o nome do plugin

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;  // não será usado midi. Voxium é um plugin de ÁUDIO.
	double getTailLengthSeconds() const override;

	int getNumPrograms() override;
	int getCurrentProgram() override; 
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	// salva/restaura o estado do plugin (Key/Scale/Harmony) via APVTS --
	// e o que faz a DAW lembrar as escolhas ao salvar/reabrir o projeto
	void getStateInformation(juce::MemoryBlock& destData) override; // serve para o estado do plugin.
	void setStateInformation(const void* data, int sizeInBytes) override;

	// exposto pro PluginEditor conseguir criar os ComboBoxAttachment
	juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }

	float getCurrentPitch() const { return currentPitch.load(); }
	float getCurrentLevel() const { return currentLevel.load(); } // DEBUGGING Purposes
	NoteInfo getCurrentNote() const { return NoteUtils::frequencyToNote(currentPitch.load()); }
	int getLastBlockSize() const { return lastBlockSize.load(); }

	bool isCurrentNoteInScale() const {
		NoteInfo note = getCurrentNote();
		if (note.midiNoteNumber < 0)
			return false;

		int noteClass = ((note.midiNoteNumber % 12) + 12) % 12;
		return ScaleUtils::isNoteInScale(noteClass, getSelectedKeyRootNote(), getSelectedScaleType());
	}

	NoteInfo getHarmonyNote() const {
		NoteInfo original = getCurrentNote();
		if (original.midiNoteNumber < 0)
			return { "", -1, 0.0f };

		int harmonyMidi = HarmonyUtils::getHarmonyNote(original.midiNoteNumber, getSelectedKeyRootNote(), getSelectedScaleType(), getHarmonyDegreeOffset());

		float harmonyFrequency = 440.0f * std::pow(2.0f, (harmonyMidi - 69) / 12.0f);
		return NoteUtils::frequencyToNote(harmonyFrequency);
	}

private:
	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	juce::AudioProcessorValueTreeState parameters;

	// ponteiros pros valores atuais dos parametros (atomicos, seguros pra
	// ler de qualquer thread) -- resolvidos uma vez no construtor
	std::atomic<float>* keyParam = nullptr;
	std::atomic<float>* scaleParam = nullptr;
	std::atomic<float>* harmonyParam = nullptr;
	std::atomic<float>* mixParam = nullptr;

	// os offsets de grau de escala continuam os mesmos de antes, so que
	// agora indexados pelo indice do parametro "harmony" (0-4), nao mais
	// setados diretamente pela UI
	static constexpr int harmonyOffsets[5] = { -2, 0, 2, 4, 7 };

	int getSelectedKeyRootNote() const { return (int)keyParam->load(); }
	ScaleType getSelectedScaleType() const { return (ScaleType)(int)scaleParam->load(); }
	int getHarmonyDegreeOffset() const { return harmonyOffsets[(int)harmonyParam->load()]; }

	PitchDetector pitchDetector;
	std::atomic<float> currentPitch{ 0.0f };  // variável atômica para armazenar a frequência detectada
	std::atomic<float> currentLevel{ 0.0f };  // DEBUGGING PURPOSES
	std::atomic<int> lastBlockSize{ 0 };

	static constexpr int pitchAnalysisSize = 2048; // janela de análise, independente do tamanho do bloco do driver
	std::vector<float> pitchAnalysisBuffer;

	PitchShifter pitchShifter;
	std::vector<float> shifterOutputBuffer;
	float smoothedPeriodInSamples = 0.0f;
	float smoothedPitchRatio = 1.0f;
	int blocksSinceLastValidPitch = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxiumAudioProcessor)
};