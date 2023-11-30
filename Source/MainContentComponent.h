#pragma once

#include <JuceHeader.h>
#include "Gui_record_play.h"

class MainContentComponent   : public juce::AudioAppComponent,
public juce::ChangeListener, public juce::Button::Listener, public juce::Timer, public juce::Slider::Listener
{
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider *slider) override;
    bool loadAudioFile(juce::File &file);
    void openFile(bool forOutput);
    
private:

    void changeState(AppState newState);
    
    void buttonClicked(juce::Button *button) override;

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton recordButton;
    juce::Slider scrubber;
    DisplayAudioWaveForm waveForm;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioTransportSource> transportSource = std::make_unique<juce::AudioTransportSource>();
//    juce::AudioTransportSource transportSource;
    AppState state;

    std::unique_ptr<AudioToFileWriter> fileWriter = std::make_unique<AudioToFileWriter>();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
