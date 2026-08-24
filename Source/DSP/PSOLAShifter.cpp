#include "PSOLAShifter.h"
#include <cmath>
#include <algorithm>

namespace {
	// janela de Hann: suaviza as bordas de um grão pra evitar cliques/descontinuidades
	// quando ele é somado (overlap-add) com os grãos vizinhos
	constexpr float pi = 3.14159265358979323846f;

	// IMPORTANTE: usa "length" no denominador (nao "length - 1"). Essa e a
	// versao "periodica" da janela Hann, que e a que garante que a soma de
	// graos sobrepostos em 50% fique perfeitamente constante (sem modulacao
	// de volume). A versao "simetrica" (com length-1) parece quase igual,
	// mas causa uma oscilacao sutil de volume a cada periodo da nota -- e
	// e exatamente essa oscilacao que soa como "ruido granulado/robotico".
	inline float hannWindow(int i, int length)
	{
		if (length <= 1)
			return 1.0f;

		return 0.5f * (1.0f - std::cos(2.0f * pi * (float)i / (float)length));
	}
}

void PSOLAShifter::prepare(int newMaxBlockSize, int newMaxPeriodSamples) {
	maxBlockSize = newMaxBlockSize;
	maxPeriodSamples = newMaxPeriodSamples;

	// o acumulador precisa ser grande o suficiente pra caber um bloco inteiro de
	// saída, mais a "cauda" de um grão inteiro que pode se estender além do bloco atual
	int bufferSize = maxBlockSize + (2 * maxPeriodSamples) + 64; // + margem de segurança
	overlapBuffer.assign((size_t)bufferSize, 0.0f);

	reset();
}

void PSOLAShifter::reset() {
	std::fill(overlapBuffer.begin(), overlapBuffer.end(), 0.0f);
	synthesisMarkCounter = 0.0f;
}

void PSOLAShifter::addGrain(const float* analysisBuffer, int analysisBufferSize,
	int grainLength, int analysisCenterIndex, int outputCenterPosition)
{
	// o grao e extraido da posicao do analysisBuffer que corresponde ao momento
	// REAL em que ele deve acontecer (nao sempre do fim absoluto do buffer!) --
	// isso evita que multiplos graos disparados no mesmo bloco peguem audio
	// identico, o que causava um efeito de "filtro pente" (comb filter) e soava
	// como robotismo/glitch repetitivo.
	int grainStartInAnalysis = analysisCenterIndex - (grainLength / 2);

	// posicao no overlapBuffer onde o grao comeca (a metade dele fica antes do centro)
	int outputStart = outputCenterPosition - (grainLength / 2);

	for (int i = 0; i < grainLength; ++i) {
		int analysisIndex = grainStartInAnalysis + i;
		int outputIndex = outputStart + i;

		// protege contra ler fora do analysisBuffer ou escrever fora do overlapBuffer
		if (analysisIndex < 0 || analysisIndex >= analysisBufferSize)
			continue;

		if (outputIndex < 0 || outputIndex >= (int)overlapBuffer.size())
			continue;

		float windowedSample = analysisBuffer[analysisIndex] * hannWindow(i, grainLength);
		overlapBuffer[(size_t)outputIndex] += windowedSample;
	}
}

void PSOLAShifter::process(const float* analysisBuffer, int analysisBufferSize, int numNewSamples,
	float periodInSamples, float pitchRatio, float* output) {
	// sem pitch detectado (silêncio/ruído) -> não há nada pra sintetizar,
	// saída em zero e reseta o estado pra evitar acumular lixo
	if (periodInSamples <= 0.0f || pitchRatio <= 0.0f) {
		for (int i = 0; i < numNewSamples; ++i)
			output[i] = 0.0f;

		// ainda assim, avanca o overlapBuffer normalmente (ele pode ter conteúdo residual
		// de um grão anterior que precisa terminar de tocar)
		for (int i = 0; i < numNewSamples; ++i)
			output[i] = (i < (int)overlapBuffer.size()) ? overlapBuffer[(size_t)i] : 0.0f;

		int remaining = (int)overlapBuffer.size() - numNewSamples;
		if (remaining > 0)
			std::copy(overlapBuffer.begin() + numNewSamples, overlapBuffer.end(), overlapBuffer.begin());

		std::fill(overlapBuffer.end() - std::min(numNewSamples, (int)overlapBuffer.size()), overlapBuffer.end(), 0.0f);

		return;
	}

	// protege contra periodos absurdos (fora da faixa que o buffer suporta),
	// e arredonda pra um numero inteiro de samples -- isso e crucial: o tamanho
	// do grao e o espacamento entre eles precisam estar perfeitamente alinhados
	// matematicamente (grainLength = 2 * period exatamente), senao a soma das
	// janelas Hann fica levemente instavel ao longo do tempo, causando uma
	// modulacao de volume periodica que soa como "vibracao"
	float clampedPeriodFloat = std::min(periodInSamples, (float)maxPeriodSamples);
	int period = (int)std::round(clampedPeriodFloat);
	period = std::max(4, period);

	int grainLength = 2 * period; // sempre par, sempre exatamente 2x o periodo

	// distancia entre disparos de graos na SAIDA (menor = pitch mais agudo)
	float synthesisPeriod = (float)period / pitchRatio;
	synthesisPeriod = std::max(4.0f, synthesisPeriod); // evita loop infinito com valores absurdos

	// dispara quantos graos forem necessarios pra cobrir esse bloco.
	// IMPORTANTE: cada grao e escrito com um deslocamento de "latencySamples" a frente
	// da posicao de entrega -- isso garante que sempre existe espaco suficiente no
	// overlapBuffer pra caber a metade "de tras" do grao, sem nunca precisar cortar
	// ela (o que causava perda de energia periodica / os "blips" no audio).
	while (synthesisMarkCounter < (float)numNewSamples)
	{

		DBG(
			"Pitch period=" << period
			<< " grain=" << grainLength
			<< " synthPeriod=" << synthesisPeriod
			<< " center=" << analysisCenterIndex
			<< " maxSafe=" << maxSafeCenter
		);

		int markPosition = (int)synthesisMarkCounter;
		int writePosition = markPosition + maxPeriodSamples; // maxPeriodSamples = latencia fixa

		int analysisCenterIndex = analysisBufferSize - numNewSamples + markPosition;

		// IMPORTANTE: um grao precisa de "grainLength/2" samples DEPOIS do seu centro
		// pra ficar completo. Se o centro calculado ficar perto demais do fim do
		// analysisBuffer, a metade de tras do grao pediria audio que ainda nao existe
		// (literalmente "do futuro"), e ficaria cortado de forma assimetrica -- isso
		// quebra a janela Hann bem naquele ponto e gera um estalo/pop audivel.
		// Por isso, limitamos o centro pra nunca chegar mais perto do fim do buffer
		// do que "grainLength/2" -- garantindo que todo grao sempre tenha conteudo
		// completo disponivel dos dois lados.
		int maxSafeCenter = analysisBufferSize - (grainLength / 2) - 1;
		analysisCenterIndex = std::min(analysisCenterIndex, maxSafeCenter);

		addGrain(analysisBuffer, analysisBufferSize, grainLength, analysisCenterIndex, writePosition);
		synthesisMarkCounter += synthesisPeriod;
	}

	// prepara a marca pro próximo bloco (relativa ao novo início de tempo)
	synthesisMarkCounter -= (float)numNewSamples;

	// entrega a parte "vencida" do acumulador como saída
	for (int i = 0; i < numNewSamples; ++i)
		output[i] = overlapBuffer[(size_t)i];

	// desloca o acumulador pra esquerda (o que sobrou continua valendo pro próximo bloco)
	int remaining = (int)overlapBuffer.size() - numNewSamples;
	if (remaining > 0)
		std::copy(overlapBuffer.begin() + numNewSamples, overlapBuffer.end(), overlapBuffer.begin());

	std::fill(overlapBuffer.end() - std::min(numNewSamples, (int)overlapBuffer.size()), overlapBuffer.end(), 0.0f);
}