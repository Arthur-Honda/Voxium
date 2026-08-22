#pragma once

#include <vector>
#include <string>
#include <array>

// Os tipos de escala suportados pelo Voxium.
// Cada uma tem um "padrao de intervalos" fixo, em semitons a partir da tonica.
enum class ScaleType {
	Major,
	NaturalMinor,
	HarmonicMinor,
	MelodicMinor,
	MajorPentatonic,
	MinorPentatonic,
	Dorian,
	Phrygian,
	Lydian,
	Mixolydian,
	Locrian
};

// Utilitario pra trabalhar com escalas musicais: dado uma tonica (key) e um
// tipo de escala, calcula quais notas (0-11, representando C, C#, D...) pertencem a ela.
class ScaleUtils {
public:
	// keyRootNote: 0 = C, 1 = C#, 2 = D ... 11 = B (mesma convencao usada no NoteUtils)
	// retorna as notas da escala como indices 0-11 (classe de nota, sem oitava)
	static std::vector<int> getScaleNotes(int keyRootNote, ScaleType scaleType);

	// verifica se uma nota (0-11) pertence a escala dada uma tonica e tipo
	static bool isNoteInScale(int noteToCheck, int keyRootNote, ScaleType scaleType);

	// nome legivel do tipo de escala, pra mostrar na UI (ex: "Major", "Natural Minor")
	static std::string getScaleName(ScaleType scaleType);

private:
	// padrao de intervalos (em semitons a partir da tonica) de cada escala
	static const std::vector<int>& getIntervalPattern(ScaleType scaleType);
};