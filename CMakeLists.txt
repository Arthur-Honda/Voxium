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

target_sources(Voxium
	PRIVATE
		Source/PluginProcessor.cpp
		Source/PluginProcessor.h
		Source/PluginEditor.cpp
		Source/PluginEditor.h
		Source/PitchDetector.cpp 
		Source/PitchDetector.h
)

juce_generate_juce_header(Voxium)

target_compile_definitions(Voxium
    PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)

target_link_libraries(Voxium 
	PRIVATE
	juce::juce_audio_utils
)