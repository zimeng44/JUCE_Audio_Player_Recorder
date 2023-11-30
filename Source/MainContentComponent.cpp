#include <JuceHeader.h>
#include "MainContentComponent.h"


MainContentComponent::MainContentComponent()
    : state(IDLE)
{
    
    // THIS IS THE CONSTRUCTOR IMPLEMENTATION
    
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.addListener(this);
    openButton.setEnabled(true);

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);
    
    addAndMakeVisible(&recordButton);
    recordButton.setButtonText("Record");
    recordButton.addListener(this);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    recordButton.setEnabled(true);
    
    addAndMakeVisible(&scrubber);
    scrubber.setEnabled(false);
    scrubber.addListener(this);
    
    addAndMakeVisible(&waveForm);

    waveForm.setEnabled(true);

    setSize(350, 300);

    formatManager.registerBasicFormats();       // [1]
    transportSource->addChangeListener(this);    // [2]

    setAudioChannels(1, 2);

}

MainContentComponent::~MainContentComponent()
{
    if (transportSource->isPlaying()) transportSource->stop();
//    readerSource.reset();
    fileWriter.reset();
    shutdownAudio();
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (state == IDLE) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    if(state == PLAYING){
        
//        if (readerSource.get() == nullptr)
//        {
//            bufferToFill.clearActiveBufferRegion();
//            return;
//        }
        
        transportSource->getNextAudioBlock(bufferToFill);
        
        waveForm.addAudioData(*bufferToFill.buffer);
        
        return;
        
    }
    
    if (state == RECORDING){
        
        auto* device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels  = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
        
        if (readerSource.get() != nullptr)
        {
            transportSource->releaseResources();
            readerSource.release();
        }
        
        waveForm.addAudioData(*bufferToFill.buffer);
        
        for (auto channel = 0; channel < maxOutputChannels; ++channel)
        {
            if ((! activeOutputChannels[channel]) || maxInputChannels == 0) //  If the maximum number of input channels is zero, or Individual output channels are inactive
            {
                bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                if (activeInputChannels[channel]) // [2]
                {
                    fileWriter->writeOutputToFile(*bufferToFill.buffer);
                    bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
                }
                else
                {
                    bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
                }
            }
        }
    }
    
}

void MainContentComponent::releaseResources()
{
    if (transportSource->isPlaying()) transportSource->stop();
    transportSource->releaseResources();
}

void MainContentComponent::resized()
{
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
    recordButton.setBounds(10, 100, getWidth() - 20, 20);
    scrubber.setBounds(10, 130, getWidth() - 20, 20);
    waveForm.setBounds(10, 160, getWidth()-20, 130);

}

void MainContentComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == transportSource.get())
    {
        if (transportSource->isPlaying())
            changeState(PLAYING);
        else
            changeState(IDLE);
    }
}

void MainContentComponent::changeState(AppState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case PLAYING:                           // [3]
                openButton.setEnabled(false);
                playButton.setEnabled(false);
                recordButton.setEnabled(false);
                stopButton.setEnabled(true);
                scrubber.setEnabled(true);
                transportSource->setPosition(0.0);
                break;

            case RECORDING:                          // [4]
                playButton.setEnabled(false);
                recordButton.setEnabled(false);
                openButton.setEnabled(false);
                stopButton.setEnabled(true);
                scrubber.setEnabled(false);
                break;

            case IDLE:                           // [5]
                openButton.setEnabled(true);
                playButton.setEnabled(false);
                recordButton.setEnabled(true);
                stopButton.setEnabled(false);
                scrubber.setEnabled(true);
                break;
        }
    }
}

void MainContentComponent::buttonClicked(juce::Button *button)
{
    if (button == &openButton){
//        transportSource->stop();
        openFile(false);
    }else if (button == &playButton){
        if(transportSource->isPlaying()) return;
        if (readerSource.get() != nullptr){
            transportSource->setPosition(0.0);
            transportSource->start();
//            readerSource->setLooping(true);
            changeState(PLAYING);
        }
    }else if (button == &stopButton){
        if(state == RECORDING){
            fileWriter->closeFile();
            changeState(IDLE);
        }else if (state == PLAYING){
            transportSource->stop();
            changeState(IDLE);
        }
    }else if (button == &recordButton){
        if(readerSource != nullptr) {
            readerSource.reset();
            stopTimer();
            if(transportSource != nullptr) transportSource.release();
        }
        openFile(true);
    }
}

void MainContentComponent::timerCallback()
{
    scrubber.setValue(transportSource->getCurrentPosition());
    if(!transportSource->isPlaying()) transportSource->setPosition(0.0);
}

void MainContentComponent::sliderValueChanged(juce::Slider *slider)
{
    transportSource->setPosition(slider->getValue());
    if (!transportSource->isPlaying()) scrubber.setEnabled(false);
}

void MainContentComponent::openFile(bool forOutput) {
       if (forOutput) {
           chooser = std::make_unique<juce::FileChooser>("Select a Wave file to record to...",
                                                         juce::File{},
                                                                            "*.wav");                     // [7]
           auto chooserFlags = juce::FileBrowserComponent::saveMode
                             | juce::FileBrowserComponent::canSelectFiles;
        
           
           chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
           {
               auto file = fc.getResult();
               if(fc.getResult() == juce::File{}) return;
               
               if(fileWriter->setup(file, 44100, 1)){
                   changeState(RECORDING);
               }
            });
       } else {
         // chooserFlags: include FileBrowserComponent::openMode
           chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                         juce::File{},
                                                             "*.wav");                     // [7]
           auto chooserFlags = juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles;
           
           chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)       // [8]
           {
               auto file = fc.getResult();
               
               if (loadAudioFile(file))
               {
                   playButton.setEnabled(true);
                   scrubber.setEnabled(true);
                   scrubber.setRange(0, transportSource->getLengthInSeconds());
                   startTimerHz(30); // 30 calls per second
               }
                else playButton.setEnabled(false);
           });
       }
}

bool MainContentComponent::loadAudioFile(juce::File &file) {
               // load and prepare the file for playing
               // Call transportSource.setSource().
               // If successful, return true;
    auto* reader = formatManager.createReaderFor(file);
    
    if (reader != nullptr)
    {
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
        transportSource.reset(new juce::AudioTransportSource);
        transportSource->setSource(newSource.get(), 0, nullptr, 0, 2);       // [12]
        transportSource->stop();
        transportSource->setSource(nullptr);
        transportSource->setSource(newSource.get(), 0, nullptr, 0, 2);
        readerSource.reset(newSource.release());
        return true;                                      // [14]
    }
    return false; // default
}
