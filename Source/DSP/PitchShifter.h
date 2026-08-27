#pragma once

#include <rubberband/RubberBandStretcher.h>
#include <vector>
#include <memory>
#include <algorithm>

// Substitui o PSOLAShifter. Usa o RubberBandStretcher completo (nao o
// RubberBandLiveShifter) em modo streaming/tempo-real, com as opcoes de
// qualidade recomendadas pela propria documentacao do Rubber Band pra voz
// com pitch mudando dinamicamente:
//
//   OptionProcessRealTime | OptionEngineFiner | OptionWindowShort |
//   OptionFormantPreserved | OptionPitchHighConsistency
//
// Trade-off consciente: essa combinacao tem mais LATENCIA que o
// RubberBandLiveShifter (que e otimizado pra latencia minima, mas soa
// mais artificial/menos humano em voz). Como o Voxium roda dentro de uma
// DAW (nao monitoramento ao vivo no palco), a latencia extra e reportada
// via getLatencySamples() e a DAW compensa automaticamente (PDC).
//
// Diferenca de uso em relacao ao LiveShifter: o RubberBandStretcher aceita
// blocos de tamanho VARIAVEL em process() (nao exige um bloco fixo como o
// LiveShifter), entao nao precisamos de FIFO de entrada -- so uma FIFO de
// saida, porque o numero de samples disponiveis via available()/retrieve()
// nao necessariamente bate com o numSamples que o host pediu naquele bloco.
class PitchShifter
{
public:
	PitchShifter() = default;

	void prepare(double sampleRate, int maxHostBlockSize)
	{
		using namespace RubberBand;

		auto options = RubberBandStretcher::OptionProcessRealTime
			| RubberBandStretcher::OptionEngineFiner
			| RubberBandStretcher::OptionWindowShort
			| RubberBandStretcher::OptionFormantPreserved
			| RubberBandStretcher::OptionPitchHighConsistency;

		// canal unico (mono), timeRatio sempre 1.0 (nunca estica tempo,
		// so muda pitch), pitchScale inicial 1.0 (sem harmonia ainda)
		stretcher = std::make_unique<RubberBandStretcher>(
			(size_t)sampleRate, (size_t)1, options, 1.0, 1.0);

		stretcher->setMaxProcessSize((size_t)maxHostBlockSize);

		latencySamples = (int)stretcher->getLatency();

		// FIFO de saida com folga generosa: precisa caber a latencia
		// interna do stretcher, mais margem pra picos de "available()"
		int fifoCapacity = (maxHostBlockSize * 4) + latencySamples + 256;
		outputFifo.assign((size_t)fifoCapacity, 0.0f);
		outputFifoFill = 0;

		retrieveScratch.assign((size_t)(maxHostBlockSize * 2 + 256), 0.0f);

		currentPitchRatio = 1.0f;
	}

	// latencia introduzida pelo shifter, em samples -- reportar isso ao
	// host via AudioProcessor::setLatencySamples() pra ativar o PDC da DAW
	int getLatencySamples() const { return latencySamples; }

	void reset()
	{
		if (stretcher != nullptr)
			stretcher->reset();

		std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
		outputFifoFill = 0;
	}

	void setPitchRatio(float ratio)
	{
		if (stretcher == nullptr)
			return;

		if (ratio != currentPitchRatio)
		{
			stretcher->setPitchScale((double)ratio);
			currentPitchRatio = ratio;
		}
	}

	void process(const float* input, float* output, int numSamples)
	{
		if (stretcher == nullptr)
		{
			std::fill(output, output + numSamples, 0.0f);
			return;
		}

		const float* inPtrs[1] = { input };
		stretcher->process(inPtrs, (size_t)numSamples, false);

		// esvazia tudo que o stretcher tiver pronto pra essa altura, e
		// acumula na FIFO de saida (pode ser mais ou menos que numSamples)
		int available = (int)stretcher->available();
		while (available > 0)
		{
			int toRetrieve = std::min(available, (int)retrieveScratch.size());
			float* outPtrs[1] = { retrieveScratch.data() };
			int retrieved = (int)stretcher->retrieve(outPtrs, (size_t)toRetrieve);

			if (retrieved <= 0)
				break;

			int spaceLeft = (int)outputFifo.size() - outputFifoFill;
			if (retrieved > spaceLeft)
				retrieved = spaceLeft; // seguranca contra overflow (nao deveria acontecer com o fifoCapacity calculado)

			if (retrieved <= 0)
				break;

			std::copy(retrieveScratch.begin(), retrieveScratch.begin() + retrieved, outputFifo.begin() + outputFifoFill);
			outputFifoFill += retrieved;

			available = (int)stretcher->available();
		}

		// entrega exatamente numSamples de saida (silencio se ainda nao
		// tiver o suficiente -- so acontece durante o "priming" inicial,
		// equivalente a latencia reportada por getLatencySamples())
		int toCopy = std::min(outputFifoFill, numSamples);
		std::copy(outputFifo.begin(), outputFifo.begin() + toCopy, output);
		if (toCopy < numSamples)
			std::fill(output + toCopy, output + numSamples, 0.0f);

		std::copy(outputFifo.begin() + toCopy, outputFifo.begin() + outputFifoFill, outputFifo.begin());
		outputFifoFill -= toCopy;
	}

private:
	std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
	int latencySamples = 0;

	std::vector<float> outputFifo;
	int outputFifoFill = 0;

	std::vector<float> retrieveScratch;

	float currentPitchRatio = 1.0f;
};