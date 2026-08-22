#pragma once

#include <string>

// Utilitario para converter uma frequencia (Hz) na nota musical mais proxima.
// Usa afinacao padrao: A4 (La central) = 440Hz.
struct NoteInfo {
	std::string name; // nome da nota musical (ex: "C4", "A#3", etc)
	int midiNoteNumber; // numero da nota MIDI (0-127)
	float centsOffset; // o quao "desafinado" esta em relacao a nota exata, em cents (-50 a +50)
};

class NoteUtils {
public:
	// converte uma frequencia em Hz pra nota mais proxima.
	// se frequencyHz for 0 ou negativo, retorna um NoteInfo vazio (name = "").
	static NoteInfo frequencyToNote(float frequencyHz);

private:
	static constexpr float a4Frequency = 440.0f;
	static constexpr int a4MidiNumber = 69;
};