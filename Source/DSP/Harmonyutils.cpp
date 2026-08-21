#include "HarmonyUtils.h"
#include <vector>
#include <cmath>
#include <climits>

int HarmonyUtils::snapToScale(int midiNote, int keyRootNote, ScaleType scaleType) {
	std::vector<int> scaleNotes = ScaleUtils::getScaleNotes(keyRootNote, scaleType);

	int noteClass = ((midiNote % 12 + 12)) % 12;

	// se a nota já pertence à escala, retorna ela mesma
	for (int scaleNote : scaleNotes) {
		if (scaleNote == noteClass)
			return midiNote;
	}

	// não pertence -> acha a nota da escala mais próxima (em qualquer direção)
	int closestNote = midiNote;
	int smallestDistance = INT_MAX;

	for (int scaleNote : scaleNotes) {
		// testa a nota da escala tanto na oitava atual quanto na de cima/baixo,
		// pra achar de fato a mais próxima em termos de distância real (não só de classe)
		for (int octaveShift = -1; octaveShift <= 1; ++octaveShift) {
			int candidate = scaleNote + (12 * ((midiNote / 12) + octaveShift));
			int distance = std::abs(candidate - midiNote);

			if (distance < smallestDistance) {
				smallestDistance = distance;
				closestNote = candidate;
			}
		}
	}
	return closestNote;
}

int HarmonyUtils::getHarmonyNote(int originalMidiNote, int keyRootNote, ScaleType scaleType, int scaleDegreeOffset) {
	// primeiro garante que estamos partindo de uma nota que pertence a escala
	int snappedNote = snapToScale(originalMidiNote, keyRootNote, scaleType);

	std::vector<int> scaleNotes = ScaleUtils::getScaleNotes(keyRootNote, scaleType);
	int notesPerOctave = (int) scaleNotes.size();

	if (notesPerOctave == 0)
		return originalMidiNote; // segurança, não deveria acontecer

	// monta a escala "espalhada" em várias oitavas, como uma lista contínua de graus,
	// pra facilitar navegar N graus pra frente/trás sem se preocupar com virada de oitava
	int noteClass = ((snappedNote % 12) + 12) % 12;
	int baseOctave = snappedNote / 12;

	int degreeIndex = -1;
	for (int i = 0; i < notesPerOctave; ++i) {
		if (scaleNotes[(size_t)i] == noteClass) {
			degreeIndex = i;
			break;
		}
	}

	if (degreeIndex == -1)
		return originalMidiNote; // segurança, não deveria acontecer após o snap

	// calcula o índice de destino (pode passsar de uma oitava pra outra)
	int totalDegreeIndex = degreeIndex + scaleDegreeOffset;

	// divisão/módulo que funciona corretamente mesmo com totalDegreeIndex negativo
	int octaveOffset = (int)std::floor((float)totalDegreeIndex / (float)notesPerOctave);
	int newDegreeIndex = ((totalDegreeIndex % notesPerOctave) + notesPerOctave) % notesPerOctave;

	int harmonyNoteClass = scaleNotes[(size_t)newDegreeIndex];
	int harmonyMidiNote = harmonyNoteClass + (12 * (baseOctave + octaveOffset));

	return harmonyMidiNote;
}