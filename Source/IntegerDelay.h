/*
  ==============================================================================

    IntegerDelay.h
    Created: 4 Jul 2024 6:14:20pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
class IntegerDelay {
public:
    IntegerDelay() {

    };
    IntegerDelay(int numChannels, int numSamples) : numSamples(numSamples) {
        ringBuffer = juce::AudioBuffer<float>(numChannels, numSamples);
    };
    void process(juce::AudioBuffer<float>& buffer) {
        if (numSamples <= 0) return;
        auto ringptr = ringBuffer.getArrayOfWritePointers();
        auto bufferptr = buffer.getArrayOfWritePointers();
        jassert(buffer.getNumChannels() == ringBuffer.getNumChannels());
        for (int i = 0; i < buffer.getNumSamples(); i++) {
            for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
                std::swap(bufferptr[channel][i], ringptr[channel][currentSample]);
            }
            ++currentSample;
            if (currentSample == numSamples) currentSample = 0;
        }
    };
    ~IntegerDelay() {

    };
private:
    juce::AudioBuffer<float> ringBuffer;
    int numSamples = 0, currentSample = 0;
};