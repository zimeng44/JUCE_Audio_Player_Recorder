/*
  ==============================================================================

    Gui_record_play.cpp
    Created: 4 Oct 2023 3:59:13pm
    Author:  Zi Meng

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Gui_record_play.h"

AudioToFileWriter::AudioToFileWriter(){
    
}

AudioToFileWriter::~AudioToFileWriter(){
    writer.reset();
    writer.release();
    fileStream.release();
}

bool AudioToFileWriter::setup(const juce::File &outputFile, int sampleRate, int numChannels)
{
    if(outputFile.exists()) outputFile.deleteFile();
    outputFile.create();
    fileStream = std::make_unique<juce::FileOutputStream>(outputFile);
//    wavFormat = std::make_unique<juce::WavAudioFormat>();
    auto wavFormat = std::make_unique<juce::WavAudioFormat>();
    writer.reset(wavFormat->createWriterFor(fileStream.get(), sampleRate, numChannels, 16, {}, 0));
    return true;
}

void AudioToFileWriter::writeOutputToFile(const juce::AudioBuffer<float>& buffer)
{
//    int ch = buffer.getNumChannels();
    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
//    writer->flush();
    fileStream->flush();
}

void AudioToFileWriter::closeFile()
{
    if(fileStream != nullptr){
        writer->flush();
        fileStream->flush();
        writer.reset();
        writer.release();
        fileStream.release();
    }
    
}

DisplayAudioWaveForm::DisplayAudioWaveForm(): audioVisualiser (1)
{
    addAndMakeVisible(audioVisualiser);
    audioVisualiser.setBufferSize(1024);
    audioVisualiser.setSamplesPerBlock(256);
    audioVisualiser.setNumChannels(1);
    audioVisualiser.setColours(juce::Colours::black, juce::Colours::white);
    audioVisualiser.setEnabled(true);
}

DisplayAudioWaveForm::~DisplayAudioWaveForm(){}

void DisplayAudioWaveForm::addAudioData(const juce::AudioBuffer<float> &buffer)
{
    audioVisualiser.pushBuffer(buffer.getArrayOfReadPointers(), 1, buffer.getNumSamples());
}

void DisplayAudioWaveForm::paint(juce::Graphics& g)
{
    g.fillAll();
}

void DisplayAudioWaveForm::resized()
{
    audioVisualiser.setBounds(getLocalBounds());
}
