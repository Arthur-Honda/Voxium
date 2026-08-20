#include "PitchDetector.h"
#include <cmath>
#include <limits>

void PitchDetector::prepare(double newSampleRate, int newBufferSize) {
	sampleRate = newSampleRate;
	bufferSize = newBufferSize;

	// o YIN trabalha com metade do buffer (janela de autocorrelacao) 
	yinBuffer.assign(bufferSize / 2, 0.0f);
}

float PitchDetector::detectPitch(const float* audioData) {
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
	int yinBufferSize = (int) yinBuffer.size();
	
	yinBuffer[0] = 1.0f; 
	float runningSum = 0.0f;

	for (int tau = 1; tau < yinBufferSize; ++tau){
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

	for (int tau = 2; tau < yinBufferSize; ++tau) {
		if (yinBuffer[tau] < threshold) {
			// desce até achar o mínimo local (mais preciso que só o primeiro ponto abaixo do threshold)
			while (tau + 1 <yinBufferSize && yinBuffer[tau+1] < yinBuffer[tau])
				++tau;

			return tau;
		}
	}
	// Não achou nenhum período confiável -> provavelmente silêncio ou ruído sem pitch claro
	return -1;
}

// Refina o tau encontrado usando interpolação parabólica (melhora bastante a precisão da frequência final)
float PitchDetector::parabolicInterpolation(int tauEstimate) {
	int yinBufferSize = (int) yinBuffer.size();

	int x0 = (tauEstimate < 1) ? tauEstimate : tauEstimate - 1;
	int x2 = (tauEstimate + 1 < yinBufferSize) ? tauEstimate + 1 : tauEstimate;

	if (x0 == tauEstimate)
		return (yinBuffer[tauEstimate] <= yinBuffer[x2]) ? (float) tauEstimate : (float) x2;

	if (x2 == tauEstimate)
		return (yinBuffer[tauEstimate] <= yinBuffer[x0]) ? (float)tauEstimate : (float)x0;

	float s0 = yinBuffer[x0];
	float s1 = yinBuffer[tauEstimate];
	float s2 = yinBuffer[x2];

	return tauEstimate + (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
}