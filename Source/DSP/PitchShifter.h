#pragma once

#include <rubberband/RubberBandLiveShifter.h>
// O caminho correto e resolvido automaticamente pelo CMake via target_link_libraries(rubberband).
#include <vector>
#include <memory>
#include <algorithm>

// Substitui o PSOLAShifter. Usa o RubberBandLiveShifter (API dedicada a
// pitch-shifting em tempo real, mais simples e com menos latencia que o
// RubberBandStretcher "geral") pra deslocar o pitch da voz captada.
//
// Diferenca importante em relacao ao PSOLAShifter antigo: o PSOLA extraia
// "graos" de um buffer de analise deslizante (pitchAnalysisBuffer, 2048
// samples). O RubberBandLiveShifter NAO funciona assim -- ele espera
// receber o audio real, em sequencia cronologica continua, exatamente como
// chega do driver. Por isso o PluginProcessor deve alimentar esse shifter
// com o bloco de audio "cru" (buffer.getReadPointer(0), numSamples), e NAO
// com o pitchAnalysisBuffer (que e so pra deteccao de pitch).
//
// Complicacao a resolver: o RubberBandLiveShifter::shift() exige um numero
// FIXO de samples por chamada (getBlockSize()), que normalmente NAO bate
// com o numSamples que o host/DAW manda a cada processBlock() (esse pode
// variar bloco a bloco). Por isso essa classe mantem duas FIFOs internas:
// uma de entrada (acumula ate ter samples suficientes pra chamar shift())
// e uma de saida (acumula o que o shift() devolve, ate ter o suficiente
// pra entregar exatamente numSamples de volta pro processBlock).
class PitchShifter
{
public:
	PitchShifter() = default;

	// sampleRate: sample rate atual do host
	// maxHostBlockSize: o maior numSamples que o processBlock() pode receber
	//                    (JUCE fornece isso em prepareToPlay)
	void prepare(double sampleRate, int maxHostBlockSize)
	{
		using namespace RubberBand;

		// Um canal (mono): Voxium trata a voz captada como uma unica fonte
		// mono, igual o PSOLAShifter fazia.
		shifter = std::make_unique<RubberBandLiveShifter>(
			(size_t)sampleRate,
			(size_t)1,
			RubberBandLiveShifter::OptionFormantPreserved);
		// OptionFormantPreserved: mantem o timbre/formantes da voz original
		// ao inves de deixar o pitch shift "esticar" o timbre tambem (o que
		// soa tipo "Chewbacca"/esquilo em desvios grandes de pitch -- o
		// mesmo tipo de artefato que o PSOLA proprio sofria).

		rbBlockSize = (int)shifter->getBlockSize();

		// FIFOs com folga suficiente pra nunca estourar entre chamadas
		int fifoCapacity = rbBlockSize + maxHostBlockSize + 64;
		inputFifo.assign((size_t)fifoCapacity, 0.0f);
		outputFifo.assign((size_t)fifoCapacity, 0.0f);
		inputFifoFill = 0;
		outputFifoFill = 0;

		rbInputScratch.assign((size_t)rbBlockSize, 0.0f);
		rbOutputScratch.assign((size_t)rbBlockSize, 0.0f);

		currentPitchRatio = 1.0f;
	}

	// zera todo o estado interno (usar ao trocar de configuracao ou parar/comecar a tocar)
	void reset()
	{
		if (shifter != nullptr)
			shifter->reset();

		std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
		std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
		inputFifoFill = 0;
		outputFifoFill = 0;
	}

	// fator de multiplicacao da frequencia (1.0 = sem mudanca, 2.0 = uma
	// oitava acima, etc) -- pode ser chamado a cada bloco, o RubberBand
	// aceita mudanca de pitch em tempo real sem reiniciar o processamento
	void setPitchRatio(float ratio)
	{
		if (shifter == nullptr)
			return;

		if (ratio != currentPitchRatio)
		{
			shifter->setPitchScale((double)ratio);
			currentPitchRatio = ratio;
		}
	}

	// input: audio mono "cru", em sequencia cronologica real (ex: buffer.getReadPointer(0))
	// output: buffer de saida, precisa ter pelo menos numSamples de tamanho
	// numSamples: numero de samples desse bloco (o numSamples do processBlock do host)
	void process(const float* input, float* output, int numSamples)
	{
		if (shifter == nullptr)
		{
			std::fill(output, output + numSamples, 0.0f);
			return;
		}

		// 1) acumula a entrada nova na FIFO de entrada
		std::copy(input, input + numSamples, inputFifo.begin() + inputFifoFill);
		inputFifoFill += numSamples;

		// 2) processa quantos blocos FIXOS de rbBlockSize couberem no que foi acumulado
		while (inputFifoFill >= rbBlockSize)
		{
			std::copy(inputFifo.begin(), inputFifo.begin() + rbBlockSize, rbInputScratch.begin());

			const float* inPtrs[1] = { rbInputScratch.data() };
			float* outPtrs[1] = { rbOutputScratch.data() };
			shifter->shift(inPtrs, outPtrs);

			std::copy(rbOutputScratch.begin(), rbOutputScratch.end(), outputFifo.begin() + outputFifoFill);
			outputFifoFill += rbBlockSize;

			// descarta da FIFO de entrada o que acabou de ser consumido
			std::copy(inputFifo.begin() + rbBlockSize, inputFifo.begin() + inputFifoFill, inputFifo.begin());
			inputFifoFill -= rbBlockSize;
		}

		// 3) entrega exatamente numSamples de saida.
		//    Nos primeiros blocos (antes do "priming" inicial do shifter),
		//    pode nao ter ainda numSamples acumulados -- nesse caso completa
		//    com silencio (é uma latencia de poucos blocos, inaudivel na pratica).
		int available = outputFifoFill;
		int toCopy = std::min(available, numSamples);

		std::copy(outputFifo.begin(), outputFifo.begin() + toCopy, output);
		if (toCopy < numSamples)
			std::fill(output + toCopy, output + numSamples, 0.0f);

		std::copy(outputFifo.begin() + toCopy, outputFifo.begin() + outputFifoFill, outputFifo.begin());
		outputFifoFill -= toCopy;
	}

private:
	std::unique_ptr<RubberBand::RubberBandLiveShifter> shifter;
	int rbBlockSize = 0;

	std::vector<float> inputFifo;
	int inputFifoFill = 0;

	std::vector<float> outputFifo;
	int outputFifoFill = 0;

	std::vector<float> rbInputScratch;
	std::vector<float> rbOutputScratch;

	float currentPitchRatio = 1.0f;
};