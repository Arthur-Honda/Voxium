#pragma once

#include <vector>

// PSOLA (Pitch Synchronous Overlap-and-Add) shifter em tempo real.
//
// Como funciona, resumido:
// - Recebe o mesmo "buffer de análise deslizante" que o PitchDetector já usa
//   (sempre contendo o áudio mais recente, até o instante "agora").
// - A cada chamada de process(), decide quantos "grãos" de áudio precisa
//   disparar nesse bloco, baseado no periodo da nota atual e no fator de
//   pitch shift desejado.
// - Cada grão e um pedaco de áudio de 2 periodos de duração, extraído do
//   fim do buffer de análise (ou seja, sempre do áudio mais recente),
//   com uma janela suave (Hann) aplicada nas bordas.
// - Os grãos são somados (overlap-add) num acumulador de saída, na posição
//   temporal correta -- mais próximos uns dos outros se o pitch está
//   subindo, mais espaçados se está descendo.
//
// Teste de sanidade: com pitchRatio = 1.0 (sem mudança de pitch), o
// resultado deveria soar quase idêntico ao áudio original -- isso confirma
// que a extração/overlap-add em si não está introduzindo artefatos, antes
// de testarmos o deslocamento de pitch de verdade.
class PSOLAShifter {
public:
	PSOLAShifter() = default;

	// maxBlockSize: o maior tamanho de bloco que process() vai receber (numNewSamples)
	// maxPeriodSamples: o maior período esperado, em samples (equivalente a frequência mínima)
	void prepare(int maxBlockSize, int maxPeriodSamples);

	// zera todo o estado interno (usar ao trocar de configuração ou parar/começar a tocar)
	void reset();

	// analysisBuffer: o buffer deslizante de áudio (mesmo formato do pitchAnalysisBuffer
	//                 do processor) -- o último sample dele e considerado "agora"
	// analysisBufferSize: tamanho desse buffer
	// numNewSamples: quantos samples de saída essa chamada precisa produzir
	//                (normalmente = buffer.getNumSamples() do processBlock)
	// periodInSamples: o período da nota atual, em samples (derivado da frequência
	//                   detectada pelo PitchDetector: periodo = sampleRate / frequencia)
	//                   se <= 0 (silêncio/sem pitch detectado), a saida fica em zero
	// pitchRatio: fator de multiplicação da frequência (1.0 = sem mudança,
	//              2.0 = uma oitava acima, 0.5 = uma oitava abaixo, etc)
	// output: buffer de saída, precisa ter pelo menos numNewSamples de tamanho
	void process(const float* analysisBuffer, int analysisBufferSize, int numNewSamples, float periodInSamples, float pitchRatio, float* output);

private:
	int maxBlockSize = 0;
	int maxPeriodSamples = 0;

	// acumulador de saída: guarda contribuições de grãos que ainda não foram "entregues" na saída
	// incluindo as que se estendem pro futuro (próxima chamada)
	std::vector<float> overlapBuffer;

	// distância (em samples) até o próximo grão precisar ser disparado.
	// persiste entre chamadas pra manter continuidade temporal.
	float synthesisMarkCounter = 0.0f;

	// aplica um grão (extraído do analysisBuffer, com janela Hann) no overlapBuffer,
	// centrado na posição indicada
	void addGrain(const float* analysisBuffer, int analysisBufferSize,
		int grainLength, int analysisCenterIndex, int outputCenterPosition);
};