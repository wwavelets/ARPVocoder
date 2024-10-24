/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "VocoderRewrite.cpp"
#include "IntegerDelay.h"
#include "Plugin.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void loadFile(const juce::File&);

    enum state {
        start,
        playing,
        stop
    } curState;

    //std::unique_ptr<VocoderProcessor> processor;
    std::unique_ptr<Plugin> plugin;
private:
    //==============================================================================
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> loadedFile;
    juce::Random random;
    int currentSample = 0;



    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release; // attack and release in milliseconds
    juce::AudioParameterFloat* carrierBandWidth; // percent
    juce::AudioParameterFloat* modulatorBandWidth; // percent
    juce::AudioParameterFloat* formant; // -12 to 12
    juce::AudioParameterFloat* mix; // -1 = carrier, 0 = plugin output, 1 = modulator



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
