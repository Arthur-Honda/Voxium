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
	FORMATS VST3
	PRODUCT_NAME "Voxium"

	SOURCES
		Source/PluginProcessor.cpp
		Source/PluginProcessor.h
		Source/PluginEditor.cpp
		Source/PluginEditor.h

)	

target_link_libraries(Voxium 
	PRIVATE
	juce::juce_audio_utils
)