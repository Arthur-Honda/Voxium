#pragma once

#include <vector>

// Detector de pitch usando o algoritmo YIN. 
// Só precisa chamar detectPitch() passando um buffer de áudio, e a sample rate.
// Retorna a frequência detectada em Hz, ou 0.0 se não detectou nada.

class PitchDetector {
public:
	PitchDetector() = default;

	void prepare(double sampleRate, int bufferSize);
	float detectPitch(const float* audioData);

private:
	double sampleRate = 44100.0;
	int bufferSize = 1024;
	float threshold = 0.15f; // Threshold do YIN: quanto menor, mais "rigoroso" na detecção mas pode perder notas mais fracas/com ruido).

	float minFrequency = 80.0f;   // Hz - grave (ex: voz masculina grave)
	float maxFrequency = 1000.0f; // Hz - agudo (ex: falsete)

	std::vector<float> yinBuffer;

	void difference(const float* audioData);
	void cumulativeMeanNormalizedDifference();
	int absoluteThreshold();
	float parabolicInterpolation(int tauEstimate);
};