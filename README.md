# Voxium 🎙️

**Real-time vocal harmonization plugin built with C++ and JUCE.**

> 🚧 **Work in Progress** — Voxium is currently under active development. Some harmony and scale features are still being refined.

Voxium is a VST3/Standalone audio plugin designed to generate real-time vocal harmonies. It analyzes the incoming vocal signal, detects its pitch, and uses musical theory to determine appropriate harmony intervals.

The goal is to create a lightweight and natural-sounding vocal harmonizer suitable for live use and music production.

## ✨ Current Features

* 🎤 **Real-time pitch detection**

  * YIN pitch detection algorithm
  * Sliding analysis window for reliable detection
  * Note and frequency readout

* 🎼 **Music theory processing**

  * Musical note detection
  * Multiple scale types — 🚧 WIP
  * Diatonic harmony generation — 🚧 WIP
  * Scale-aware harmony intervals

* 🔊 **Real-time pitch shifting**

  * Powered by [Rubber Band](https://breakfastquay.com/rubberband/)
  * Formant-preserving processing
  * Designed for natural-sounding vocal harmonies

* 🎛️ **Custom interface**

  * Dark visual theme
  * Real-time pitch information
  * Key, scale and harmony controls

* 🔌 **Plugin formats**

  * VST3
  * Standalone application
  * Designed for use with DAWs such as Ableton Live and FL Studio  — 🚧 WIP

## 🛠️ Tech Stack

* **C++**
* **JUCE**
* **CMake**
* **Rubber Band**
* **DSP / Real-time Audio Processing**
* **Music Theory / Digital Signal Processing**

## 🧠 How It Works

The current processing pipeline can be summarized as:

```text
Vocal Input
     │
     ▼
Pitch Detection
     │
     ▼
Note Detection
     │
     ▼
Scale & Harmony Processing
     │
     ▼
Calculate Harmony Pitch
     │
     ▼
Rubber Band Pitch Shifting
     │
     ▼
Harmonized Vocal Output
```

The pitch detector analyzes the incoming audio and estimates its fundamental frequency. The detected frequency is converted into a musical note, which is then used together with the selected musical context to determine the target harmony pitch.

The target pitch is finally processed through the pitch-shifting stage to generate the additional vocal voice.

## 📁 Project Structure

```text
Source/
├── PluginProcessor.h/.cpp
├── PluginEditor.h/.cpp
│
├── DSP/
│   └── PitchDetector.h/.cpp
│
└── Theory/
    ├── NoteUtils.h/.cpp
    ├── ScaleUtils.h/.cpp
    └── HarmonyUtils.h/.cpp
```

### DSP

Audio signal processing and analysis components.

### Theory

Music theory and symbolic pitch processing, separated from the audio DSP layer.

## 🚧 Current Status

**~50% complete**

The core audio and harmony pipeline is already functional, but Voxium is still under development.

Current work includes:

* Improving harmony accuracy
* Refining scale handling
* Improving sound quality
* UI polish
* Stability and real-time performance improvements
* Further testing with different voices and musical contexts

## 🎯 Future Plans

* More robust scale and harmony handling
* Improved pitch tracking
* Additional harmony modes
* Better control over harmony voices
* Further latency and performance optimization
* More extensive testing with real-world vocal recordings
* Final UI/UX refinement

## 📸 Demo

*Demo coming soon.*

## 📄 License

This project is proprietary and is not licensed for redistribution, modification, or commercial use.
