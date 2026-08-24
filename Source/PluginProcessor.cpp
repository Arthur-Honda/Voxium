#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

VoxiumAudioProcessor::VoxiumAudioProcessor()
	: AudioProcessor(BusesProperties()
		#if ! JucePlugin_IsMidiEffect
			#if ! JucePlugin_IsSynth
				.withInput("Input", juce::AudioChannelSet::stereo(), true)
			#endif
				.withOutput("Output", juce::AudioChannelSet::stereo(), true)
		#endif
	),
	parameters(*this, nullptr, "PARAMETERS", {})
{}
	
	VoxiumAudioProcessor::~VoxiumAudioProcessor() {}

// ==============================================================================

const juce::String VoxiumAudioProcessor::getName() const {
	return JucePlugin_Name; // retorna o nome do plugin (Voxium)
}

bool VoxiumAudioProcessor::acceptsMidi() const {
	#if JucePlugin_WantsMidiInput
		return true;
	#else
		return false;
	#endif
}

bool VoxiumAudioProcessor::producesMidi() const {
	#if JucePlugin_ProducesMidiOutput
		return true;
	#else
		return false;
	#endif
}

bool VoxiumAudioProcessor::isMidiEffect() const {
	#if JucePlugin_IsMidiEffect
		return true;
	#else
		return false;
	#endif
}

double VoxiumAudioProcessor::getTailLengthSeconds() const {
	return 0.0;
}

int VoxiumAudioProcessor::getNumPrograms() {
	return 1;
}

int VoxiumAudioProcessor::getCurrentProgram() {
	return 0;
}

void VoxiumAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }

const juce::String VoxiumAudioProcessor::getProgramName(int index) {
	juce::ignoreUnused(index);
	return{};
}

void VoxiumAudioProcessor::changeProgramName(int index, const juce::String& newName) {
	juce::ignoreUnused(index, newName);
}

// ==============================================================================

void VoxiumAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	juce::ignoreUnused(samplesPerBlock);

	pitchDetector.prepare(sampleRate, pitchAnalysisSize);
	pitchAnalysisBuffer.assign(pitchAnalysisSize, 0.0f);

	// tamanho generoso, desacoplado do tamanho de bloco real do driver
	// (mesmo raciocínio já aplicado no pitchAnalysisBuffer)
	int maxPeriodForPsola = (int)(sampleRate / 70.0) + 16; // 80Hz = freq. mín. esperada
	psolaShifter.prepare(8192, maxPeriodForPsola);
	psolaOutputBuffer.assign(8192, 0.0f);
}

void VoxiumAudioProcessor::releaseResources() {} // liberar recursos que foram alocados/necessários durante o processamento.

bool VoxiumAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
	#if JucePlugin_IsMidiEffect 
		juce::ignoreUnused(layouts);
		return true;
	#else
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
	#endif

		return true;
	#endif
}

void VoxiumAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
											juce::MidiBuffer& midiMessages) {
	juce::ignoreUnused(midiMessages);

	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());	

	for (int channel = 0; channel < totalNumInputChannels; ++channel) {
		auto* channelData = buffer.getWritePointer(channel);
		juce::ignoreUnused(channelData);
	}

	if (totalNumInputChannels > 0) {
		auto* firstChannelData = buffer.getReadPointer(0);
		int numSamples = buffer.getNumSamples();

		lastBlockSize.store(numSamples);

		if (numSamples >= pitchAnalysisSize) {
			std::copy(firstChannelData + numSamples - pitchAnalysisSize, firstChannelData + numSamples, pitchAnalysisBuffer.begin());
		}
		else {
			std::copy(pitchAnalysisBuffer.begin() + numSamples, pitchAnalysisBuffer.end(), pitchAnalysisBuffer.begin());
			std::copy(firstChannelData, firstChannelData + numSamples, pitchAnalysisBuffer.end() - numSamples);
		}

		float detectedFrequency = pitchDetector.detectPitch(pitchAnalysisBuffer.data());

		if (detectedFrequency > 0.0f) {
			currentPitch.store(detectedFrequency);
		}

		// --- PSOLA: gera a voz de harmonia deslocada ---
		float rawPeriodInSamples = (detectedFrequency > 0.0f) ? (float)(getSampleRate() / detectedFrequency) : 0.0f;

		// quantos blocos de tolerancia antes de considerar "silencio de verdade"
		// (aumentado bastante pra teste -- ~400ms)
		int holdBlocks = (int)(0.4 * getSampleRate() / (double)numSamples) + 1;

		if (rawPeriodInSamples > 0.0f)
		{
			const float smoothingAmount = 0.2f;
			smoothedPeriodInSamples += (rawPeriodInSamples - smoothedPeriodInSamples) * smoothingAmount;
			blocksSinceLastValidPitch = 0;
		}
		else
		{
			blocksSinceLastValidPitch++;

			// deteccao falhou nesse bloco, mas ainda estamos dentro da "tolerancia" ->
			// continua usando o ultimo periodo valido conhecido, ao inves de silenciar
			if (blocksSinceLastValidPitch > holdBlocks)
				smoothedPeriodInSamples = 0.0f; // silencio de verdade
		}

		float periodInSamples = smoothedPeriodInSamples;

		// calcula o pitchRatio de verdade, baseado na harmonia escolhida
		float pitchRatio = 1.0f;

		if (periodInSamples > 0.0f) {
			float smoothedFrequency = (float)(getSampleRate() / periodInSamples);
			NoteInfo originalNote = NoteUtils::frequencyToNote(smoothedFrequency);

			if (originalNote.midiNoteNumber >= 0) {
				if (harmonyDegreeOffset == 0)
				{
					// Unison: sempre exatamente o pitch original, sem nenhum
					// processamento de escala (evita que o "encaixe" na escala
					// (snapToScale) oscile por causa de pequenas variacoes
					// naturais da voz, o que causava uma modulacao rapida de
					// pitch soando como ruido/whoosh)
					pitchRatio = 1.0f;
				}
				else {
					int harmonyMidiNote = HarmonyUtils::getHarmonyNote(
						originalNote.midiNoteNumber, selectedKeyRootNote, selectedScaleType, harmonyDegreeOffset);

					int semitoneDifference = harmonyMidiNote - originalNote.midiNoteNumber;
					pitchRatio = std::pow(2.0f, (float)semitoneDifference / 12.0f);
				}
			}
		}

		if ((int)psolaOutputBuffer.size() < numSamples)
			psolaOutputBuffer.assign((size_t)numSamples, 0.0f);

		psolaShifter.process(pitchAnalysisBuffer.data(), pitchAnalysisSize, numSamples,
			periodInSamples, pitchRatio, psolaOutputBuffer.data());

		// substitui a saida pelo resultado do PSOLA (pra ouvir SO ele, sem o dry misturado)
		// TESTE: reduzindo o volume pra confirmar se a distorcao e por causa de clipping
		const float debugGainReduction = 0.5f;

		for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
			auto* channelOut = buffer.getWritePointer(channel);
			for (int i = 0; i < numSamples; ++i)
				channelOut[i] = psolaOutputBuffer[(size_t)i] * debugGainReduction;
		}
	}
}

// ==============================================================================

bool VoxiumAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* VoxiumAudioProcessor::createEditor() {
	return new VoxiumAudioProcessorEditor (*this);
}

// ==============================================================================

void VoxiumAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	juce::ignoreUnused(destData);
}

void VoxiumAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	juce::ignoreUnused(data, sizeInBytes);
}

// ==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new VoxiumAudioProcessor();
}