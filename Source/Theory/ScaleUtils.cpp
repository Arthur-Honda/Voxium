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
		// soma o intervalo a tonica, e usa modulo 12 pra "dar a volta" na roda cromatica
		// (ex: se a tonica for A=9 e o intervalo for 4, 9+4=13 -> 13%12=1 -> C#)
		int note = (keyRootNote + interval) % 12;
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