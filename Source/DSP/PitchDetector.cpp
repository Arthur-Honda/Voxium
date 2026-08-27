#include "PitchDetector.h"
#include <cmath>
#include <limits>
#include <algorithm>

void PitchDetector::prepare(double newSampleRate, int newBufferSize) {
	sampleRate = newSampleRate;
	bufferSize = newBufferSize;

	// o YIN trabalha com metade do buffer (janela de autocorrelacao) 
	yinBuffer.assign(bufferSize / 2, 0.0f);
}

float PitchDetector::detectPitch(const float* audioData) {
	// portão de silêncio: se o áudio tá muito baixo, nem tenta detectar
	int yinBufferSize = (int)yinBuffer.size();
	float energy = 0.0f;

	for (int i = 0; i < yinBufferSize; ++i)
		energy += audioData[i] * audioData[i];

	energy /= (float)yinBufferSize;

	if (energy < 0.00001f)
		return 0.0f;

	difference(audioData);
	cumulativeMeanNormalizedDifference();

	int tauEstimate = absoluteThreshold(); // não achou um período válido -> não tem pitch confiável nesse bloco.
	if (tauEstimate == -1)
		return 0.0f;

	float betterTau = parabolicInterpolation(tauEstimate);

	if (betterTau <= 0.0f)
		return 0.0f;

	return (float)(sampleRate / betterTau);
}

// Função de diferença (compara o sinal com versões atrasadas de si mesmo)
void PitchDetector::difference(const float* audioData) {
	int yinBufferSize = (int)yinBuffer.size();

	for (int tau = 0; tau < yinBufferSize; ++tau)
		yinBuffer[tau] = 0.0f;

	for (int tau = 0; tau < yinBufferSize; ++tau) {
		for (int i = 0; i < yinBufferSize; ++i) {
			float delta = audioData[i] - audioData[i + tau];
			yinBuffer[tau] += delta * delta;
		}
	}
}

// Normaliza a função de diferença (CMND - Cumulative Mean Normalized Difference)
void PitchDetector::cumulativeMeanNormalizedDifference() {
	int yinBufferSize = (int)yinBuffer.size();

	yinBuffer[0] = 1.0f;
	float runningSum = 0.0f;

	for (int tau = 1; tau < yinBufferSize; ++tau) {
		runningSum += yinBuffer[tau];

		if (runningSum == 0.0f)
			yinBuffer[tau] = 1.0f;
		else
			yinBuffer[tau] *= tau / runningSum;
	}
}

// Acha o primeiro tau (período) que fica abaixo do threshold -> isso indica um periodo repetitivo forte o suficiente para ser considerado pitch.
int PitchDetector::absoluteThreshold() {
	int yinBufferSize = (int)yinBuffer.size();

	// so busca taus que correspondem a frequencias dentro da faixa vocal esperada
	int minTau = (int)(sampleRate / maxFrequency);
	int maxTau = (int)(sampleRate / minFrequency);

	minTau = std::max(2, minTau);
	maxTau = std::min(yinBufferSize - 1, maxTau);

	for (int tau = minTau; tau <= maxTau; ++tau) {
		if (yinBuffer[tau] < threshold) {
			// desce até achar o mínimo local (mais preciso que só o primeiro ponto abaixo do threshold)
			while (tau + 1 <= maxTau && yinBuffer[tau + 1] < yinBuffer[tau])
				++tau;

			// --- Correcao de erro de oitava (octave error) ---
			// A busca acima vai do tau MENOR (frequencia mais aguda) pro
			// MAIOR (mais grave), e para no primeiro que passa no threshold.
			// Isso e o comportamento certo pra evitar o erro classico de
			// "oitava PRA BAIXO" -- mas deixa a porta aberta pro erro
			// oposto: se o segundo harmonico da nota (que tem
			// exatamente a METADE do periodo do fundamental) for forte o
			// suficiente, o CMND dele pode passar no threshold ANTES da
			// busca chegar no periodo verdadeiro -- e ai a nota inteira
			// e detectada uma OITAVA ACIMA do que realmente foi cantado.
			// Isso fica mais provavel em notas mais agudas, onde o
			// periodo fundamental e curto e o do segundo harmonico
			// (metade disso) ainda cabe dentro da faixa de busca.
			//
			// Verificacao: o periodo verdadeiro, se for esse erro, e o
			// DOBRO do tau encontrado. Se esse tau dobrado tambem for um
			// candidato razoavelmente bom (CMND baixo o suficiente),
			// preferimos ele -- e bem mais provavel ser o fundamental
			// real do que o segundo harmonico sozinho.
			int doubledTau = tau * 2;
			if (doubledTau <= maxTau && yinBuffer[doubledTau] < threshold * 1.5f)
			{
				// desce ate o minimo local tambem no candidato dobrado
				while (doubledTau + 1 <= maxTau && yinBuffer[doubledTau + 1] < yinBuffer[doubledTau])
					++doubledTau;

				return doubledTau;
			}

			return tau;
		}
	}


	// USE ABAIXO PARA DEBUGAR O YIN (mostra o tau e o valor do yinBuffer[tau] que passou do threshold)
	// 
	//for (int tau = 2; tau < yinBufferSize; ++tau) {
	//	if (yinBuffer[tau] < threshold) {
	//		// desce até achar o mínimo local (mais preciso que só o primeiro ponto abaixo do threshold)
	//		while (tau + 1 < yinBufferSize && yinBuffer[tau + 1] < yinBuffer[tau])
	//			++tau;

	//		return tau;
	//	}
	//}

	// Não achou nenhum período confiável -> provavelmente silêncio ou ruído sem pitch claro
	return -1;
}

// Refina o tau encontrado usando interpolação parabólica (melhora bastante a precisão da frequência final)
float PitchDetector::parabolicInterpolation(int tauEstimate) {
	int yinBufferSize = (int)yinBuffer.size();

	int x0 = (tauEstimate < 1) ? tauEstimate : tauEstimate - 1;
	int x2 = (tauEstimate + 1 < yinBufferSize) ? tauEstimate + 1 : tauEstimate;

	if (x0 == tauEstimate)
		return (yinBuffer[tauEstimate] <= yinBuffer[x2]) ? (float)tauEstimate : (float)x2;

	if (x2 == tauEstimate)
		return (yinBuffer[tauEstimate] <= yinBuffer[x0]) ? (float)tauEstimate : (float)x0;

	float s0 = yinBuffer[x0];
	float s1 = yinBuffer[tauEstimate];
	float s2 = yinBuffer[x2];

	float denominator = 2.0f * (2.0f * s1 - s2 - s0);

	// se o denominador for perto demais de zero, a interpolação fica instável
	// (pode gerar valores absurdos) -> nesse caso, usa o tau original sem refinar
	if (std::abs(denominator) < 1e-9f)
		return (float)tauEstimate;

	return tauEstimate + (s2 - s0) / denominator;
}