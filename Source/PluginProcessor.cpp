#include "PluginProcessor.h"
#include "PluginEditor.h"

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
	pitchDetector.prepare(sampleRate, samplesPerBlock);
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
											juce::MidiBuffer& midiMessages)
{
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
		float detectedFrequency = pitchDetector.detectPitch(firstChannelData);

		if (detectedFrequency > 0.0f) {
			currentPitch.store(detectedFrequency);
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