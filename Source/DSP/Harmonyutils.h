#pragma once

#include "ScaleUtils.h"

// Utilitário pra calcular notas de harmonia DENTRO de uma escala (diatonicamente),
// não apenas somando semitons fixos. Isso é o que faz a harmonia soar "correta"
// musicalmente: uma terça acima de C em C Major e E (4 semitons), mas uma terça
// acima de D na mesma escala e F (so 3 semitons) -- o intervalo em si muda
// dependendo de onde você está na escala.
class HarmonyUtils
{
public:
	// Calcula a nota harmônica, dado:
	// - originalMidiNote: a nota MIDI que esta sendo cantada (ex: 60 = C4)
	// - keyRootNote: a tonica escolhida (0 = C, 1 = C#, etc)
	// - scaleType: a escala escolhida
	// - scaleDegreeOffset: quantos "degraus" da escala subir (ou descer, se negativo)
	//     Exemplos: 2 = terça acima (pula 2 notas na escala), 4 = quinta acima, 7 = oitava
	//
	// Se a nota original não pertencer exatamente a escala, ela é "encaixada"
	// (snapped) pra nota mais próxima da escala antes de calcular a harmonia --
	// isso evita notas dissonantes caso a detecção de pitch pegue algo levemente
	// fora da escala (ex: uma nota de passagem ou uma leve imprecisão vocal).
	static int getHarmonyNote(int originalMidiNote, int keyRootNote, ScaleType scaleType, int scaleDegreeOffset);

	// Encaixa uma nota MIDI pra nota mais próxima que pertence a escala
	// (usado internamente por getHarmonyNote, mas exposto pra ser reutilizável)
	static int snapToScale(int midiNote, int keyRootNote, ScaleType scaleType);
};