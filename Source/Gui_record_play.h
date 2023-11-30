/*
  ==============================================================================

    Gui_record_play.h
    Created: 4 Oct 2023 3:58:06pm
    Author:  Zi Meng

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum AppState {
    IDLE,
    PLAYING,
    RECORDING
};

class AudioToFileWriter
{
public:
    AudioToFileWriter();
    ~AudioToFileWriter();
    bool setup(const juce::File& outputFile, int sampleRate, int numChannels);
    void writeOutputToFile(const juce::AudioBuffer<float>& buffer);
    void closeFile();
    
private:
    std::unique_ptr<juce::FileOutputStream> fileStream;
    std::unique_ptr<juce::AudioFormatWriter> writer;
};

class DisplayAudioWaveForm : public juce::Component
{
public:
    DisplayAudioWaveForm();
    ~DisplayAudioWaveForm() override;
    void addAudioData(const juce::AudioBuffer<float>& buffer);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::AudioVisualiserComponent audioVisualiser;
};
