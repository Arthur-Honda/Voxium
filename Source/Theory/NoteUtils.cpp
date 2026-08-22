#include "NoteUtils.h"
#include <cmath>
#include <array>

NoteInfo NoteUtils::frequencyToNote(float frequencyHz) {
	if(frequencyHz <= 0.0f)
		return NoteInfo{ "", -1, 0.0f };

	// quantos semitons de distância essa frequência está do A4 (440Hz)?
	// fórmula padrão: semitons = 12*log2(freq/freqReferencia)
	float semitonesFromA4 = 12.0f * std::log2(frequencyHz / a4Frequency);

	// arredonda para o semitom mais próximo -> essa é a nota "real"
	int roundedSemitones = (int)std::round(semitonesFromA4);

	// diferença entre a frequência real e a nota exata, em cents (100 cents = 1 semitom)
	// útil pra saber o quão afinado/desafinado a voz está
	float centsOffset = (semitonesFromA4 - (float)roundedSemitones) * 100.0f;

	int midiNoteNumber = a4MidiNumber + roundedSemitones;

	// nomes das notas em ordem cromatica, começando do C (Dó)
	static const std::array<std::string, 12> noteNames = {
		"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
	};

	// MIDI note 60 = C4 (Dó central), A partir disso da pra calcular.
	// o índice da nota (0-11) e a oitava.

	int noteIndex = ((midiNoteNumber % 12) + 12) % 12; // garante que fique entre 0-11 mesmo que midiNoteNumber seja negativo
	int octave = (midiNoteNumber / 12) - 1;

	std::string noteName = noteNames[(size_t)noteIndex] + std::to_string(octave);
	
	return NoteInfo{ noteName, midiNoteNumber, centsOffset };
}