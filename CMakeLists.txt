cmake_minimum_required(VERSION 3.22)

project(Voxium VERSION 0.1.0)

add_subdirectory(H:/Dev/JUCE JUCE)

juce_add_plugin(Voxium
	COMPANY_NAME "Voxium"
	IS_SYNTH FALSE
	NEEDS_MIDI_INPUT FALSE
	NEEDS_MIDI_OUTPUT FALSE 
	IS_MIDI_EFFECT FALSE
	COPY_PLUGIN_AFTER_BUILD TRUE
	PLUGIN_MANUFACTURER_CODE Vxum
	PLUGIN_CODE Vox1
	FORMATS VST3 Standalone
	VST3_CAN_REPLACE_VST2 FALSE
	PRODUCT_NAME "Voxium"
)	

# --- Rubber Band: baixa so o codigo-fonte (a lib atual nao tem CMakeLists.txt
# proprio, o build oficial deles e via Meson) e compila o arquivo unico
# single/RubberBandSingle.cpp direto como parte do Voxium. Esse arquivo e
# fornecido pelo proprio projeto Rubber Band exatamente pra esse cenario:
# embutir a lib inteira num unico compilation unit, sem precisar de build
# system separado nem dependencias externas (usa FFT e resampler embutidos).
include(FetchContent)
FetchContent_Declare(
    rubberband
    GIT_REPOSITORY https://github.com/breakfastquay/rubberband.git
    GIT_TAG default
)
FetchContent_Populate(rubberband)
# (usamos Populate, nao MakeAvailable: MakeAvailable tentaria rodar
# add_subdirectory, mas nao ha CMakeLists.txt nesse repo pra isso)

target_sources(Voxium
	PRIVATE
		Source/PluginProcessor.cpp
		Source/PluginProcessor.h
		Source/PluginEditor.cpp
		Source/PluginEditor.h
		Source/DSP/PitchDetector.cpp 
		Source/DSP/PitchDetector.h
		Source/Theory/NoteUtils.cpp 
		Source/Theory/NoteUtils.h
		Source/Theory/ScaleUtils.cpp
		Source/Theory/ScaleUtils.h
		Source/Theory/HarmonyUtils.cpp
		Source/Theory/HarmonyUtils.h
		Source/DSP/PitchShifter.h
		${rubberband_SOURCE_DIR}/single/RubberBandSingle.cpp
)

juce_generate_juce_header(Voxium)

target_compile_definitions(Voxium
    PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
    JUCE_DIRECT2D=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Necessario pra nosso PitchShifter.h achar <rubberband/RubberBandLiveShifter.h>
# (o header publico da lib fica em <rubberband_SOURCE_DIR>/rubberband/*.h)
target_include_directories(Voxium PRIVATE ${rubberband_SOURCE_DIR})

target_link_libraries(Voxium 
	PRIVATE
	juce::juce_audio_utils
)