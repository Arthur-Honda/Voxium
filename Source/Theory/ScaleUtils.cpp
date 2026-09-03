#include "ScaleUtils.h"

const std::vector<int>& ScaleUtils::getIntervalPattern(ScaleType scaleType)
{
	// cada escala e definida pela distancia (em semitons) de cada nota ate a tonica.
	// ex: Major = 0,2,4,5,7,9,11 -> tonica, tom, tom, semitom, tom, tom, tom, (semitom de volta pra tonica)
	static const std::vector<int> major = { 0, 2, 4, 5, 7, 9, 11 };
	static const std::vector<int> naturalMinor = { 0, 2, 3, 5, 7, 8, 10 };
	static const std::vector<int> harmonicMinor = { 0, 2, 3, 5, 7, 8, 11 };
	static const std::vector<int> melodicMinor = { 0, 2, 3, 5, 7, 9, 11 };

	switch (scaleType) {
	case ScaleType::Major:             return major;
	case ScaleType::NaturalMinor:      return naturalMinor;
	case ScaleType::HarmonicMinor:     return harmonicMinor;
	case ScaleType::MelodicMinor:      return melodicMinor;
	default:                           return major;
	}
}

std::vector<int> ScaleUtils::getScaleNotes(int keyRootNote, ScaleType scaleType) {
	const std::vector<int>& pattern = getIntervalPattern(scaleType);

	std::vector<int> scaleNotes;
	scaleNotes.reserve(pattern.size());

	for (int interval : pattern) {
		// IMPORTANTE: NAO aplica %12 aqui. O padrao de intervalos (pattern)
		// ja e estritamente crescente (0,2,4,5,7,9,11...), entao
		// keyRootNote + interval tambem e SEMPRE estritamente crescente,
		// independente da tonica -- so pode passar de 12 (ex: root=5,
		// ultimo intervalo=11 -> 16), nunca "dar a volta" no meio da lista.
		//
		// Se aplicassemos %12 aqui (como era antes), a lista deixava de
		// ser crescente pra qualquer tonica != C (ex: Fa Maior virava
		// {5,7,9,10,0,2,4} -- os ultimos valores menores que os primeiros,
		// mesmo sendo notas mais agudas). O HarmonyUtils depende dessa
		// lista ser crescente pra calcular em qual oitava a harmonia cai;
		// quebrar isso fazia a harmonia sair uma oitava errada em
		// qualquer tonica que nao fosse C.
		//
		// Callers que precisam da CLASSE da nota (0-11) devem aplicar
		// %12 no valor retornado (ver isNoteInScale abaixo).
		int note = keyRootNote + interval;
		scaleNotes.push_back(note);
	}

	return scaleNotes;
}

bool ScaleUtils::isNoteInScale(int noteToCheck, int keyRootNote, ScaleType scaleType) {
	std::vector<int> scaleNotes = getScaleNotes(keyRootNote, scaleType);

	for (int note : scaleNotes) {
		if (note == noteToCheck)
			return true;
	}

	return false;
}

std::string ScaleUtils::getScaleName(ScaleType scaleType)
{
	switch (scaleType)
	{
	case ScaleType::Major:            return "Major";
	case ScaleType::NaturalMinor:      return "Natural Minor";
	case ScaleType::HarmonicMinor:     return "Harmonic Minor";
	case ScaleType::MelodicMinor:      return "Melodic Minor";
	default:                           return "Unknown";
	}
}