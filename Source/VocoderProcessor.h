/*
  ==============================================================================

    VocoderProcessor.h
    Created: 6 Jun 2024 6:35:08pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "IntegerDelay.h"


class VocoderProcessor {
public:


    VocoderProcessor(juce::AudioParameterFloat* attack, juce::AudioParameterFloat* release, juce::AudioParameterFloat*lowcut, juce::AudioParameterFloat*highcut) : attack(attack), release(release), lowcut(lowcut), highcut(highcut) {
        initialize();
    };

    ~VocoderProcessor() {
        if (thread.valid())
            try {
            thread.get();
        }
        catch (std::exception& e) {
            DBG(e.what());
            jassertfalse;
        }
    }

    void setSampleRate(float rate) {
        sampleRate = rate;
        initialize();
    };

    void setBandWidth(float width) { // should be a multiplier ;; 2^(1/12) is default :3
        bandWidth = width;
        initialize();
    };

    void process(juce::AudioBuffer<float>& carrier, const juce::AudioBuffer<float>& modulator) {
        //load up 2048 samples before processing
        for (int i = 0; i < carrier.getNumSamples(); i++) {
            carrierBufferTemp.setSample(0, bufferIndex, carrier.getSample(0, i));
            carrierBufferTemp.setSample(1, bufferIndex, carrier.getSample(1, i));
            modulatorBufferTemp.setSample(0, bufferIndex, modulator.getSample(0, i));
            modulatorBufferTemp.setSample(1, bufferIndex, modulator.getSample(1, i));

            bufferIndex += 1;
            if (bufferIndex == samplesPerBlock) {
                //wait until processPrivate is done
                if (thread.valid()) {
                    try {
                        thread.get();
                    }
                    catch (std::exception& e) {
                        DBG(e.what());
                        jassertfalse;
                    }
                }

                std::swap(processedBufferTemp, processedBuffer);
                std::swap(carrierBufferTemp, carrierBuffer);
                std::swap(modulatorBufferTemp, modulatorBuffer);
                thread = std::async(std::launch::async, [&]() {
                    processPrivate(carrierBuffer, modulatorBuffer);
                    });
                bufferIndex = 0;
            }
            carrier.setSample(0, i, processedBufferTemp.getSample(0, bufferIndex));
            carrier.setSample(1, i, processedBufferTemp.getSample(1, bufferIndex));
        }
    };

    std::vector<float>& getEnvelope();
    int getNumBands();

private:
    //structs
    struct band;
    struct FFTGroup;
    struct Oversampler2X;


    //parameters
    juce::AudioParameterFloat *attack;
    juce::AudioParameterFloat *release; 
    juce::AudioParameterFloat* lowcut;
    juce::AudioParameterFloat* highcut;

    //constants
    float sampleRate = 48'000;
    float bandWidth = pow(2.0, 1.0 / 12.0);
    const float nyquist = 2.5f; // downsampling
    const float eps = 0.1f; // epsilon
    float minLowCutoff = 2.0f;
    float maxHighCutoff = 20'000.0f;
    const int startStage = 4;
    const int numStages = 11;
    int samplesPerBlock = (1 << numStages);
    float latencyInSamples = 0;
    
    //buffers
    juce::AudioBuffer<float> carrierBuffer, modulatorBuffer, processedBuffer, temporaryBuffer;
    juce::AudioBuffer<float> carrierBufferTemp, modulatorBufferTemp, processedBufferTemp;
    std::vector<juce::AudioBuffer<float>> stageBuffer;
    std::vector<IntegerDelay> stageDelay;
    int bufferIndex = 0;

    //thread
    std::future<void> thread;

    //oversamplers
    std::vector<Oversampler2X> carrierOversamplers, modulatorOversamplers;

    //FFTGroups / envelopes
    //TODO: FFTGroups / envelopes
    std::vector<float> envelope;


    struct band {
        band(const int& numSamples, const double& sampleRate, const float& low, const float& high) {
            float bandwidth = (high-low);
            auto filter = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod(bandwidth / 2.0f, sampleRate, numSamples-1, juce::dsp::FilterDesign<float>::WindowingMethod::hann)->getRawCoefficients();
            //fft the shit out of it or something?
        };
    };

    //mono
    struct FFTGroup {
        std::vector<band> bands;
    };


    struct Oversampler2X { // stereo
        juce::AudioBuffer<float> upsampled, filtered, downsampled;
        int numSamples = 0;
        float sampleRate = 0;
        int delay = 0;

        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> lowpass;
        std::unique_ptr<juce::dsp::Convolution> filter1, filter2;
        Oversampler2X(int numSamples, float sampleRate, juce::dsp::FIR::Coefficients<float>::Ptr& halfband) : numSamples(numSamples), sampleRate(sampleRate) {
            //DBG(numSamples << " " <<  halfband->getFilterOrder());

            int filterSize = halfband->getFilterOrder()+1;
            delay = halfband->getFilterOrder() / 2;
            filter1 = std::make_unique<juce::dsp::Convolution>(juce::dsp::Convolution::NonUniform{ std::min(filterSize, 256) });
            filter2 = std::make_unique<juce::dsp::Convolution>(juce::dsp::Convolution::NonUniform{ std::min(filterSize, 256) });
            //delay += std::min(filterSize, 256);

            juce::AudioBuffer<float> halfbandBuffer = juce::AudioBuffer<float>(2, filterSize);
            memcpy(halfbandBuffer.getWritePointer(0), halfband->getRawCoefficients(), sizeof(float) * filterSize);
            memcpy(halfbandBuffer.getWritePointer(1), halfband->getRawCoefficients(), sizeof(float) * filterSize);

            filter1->loadImpulseResponse(std::move(halfbandBuffer), static_cast<double>(sampleRate), juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no); 

            halfbandBuffer = juce::AudioBuffer<float>(2, filterSize);
            memcpy(halfbandBuffer.getWritePointer(0), halfband->getRawCoefficients(), sizeof(float) * filterSize);
            memcpy(halfbandBuffer.getWritePointer(1), halfband->getRawCoefficients(), sizeof(float) * filterSize);

            filter2->loadImpulseResponse(std::move(halfbandBuffer), static_cast<double>(sampleRate), juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
            

            filter1->prepare(juce::dsp::ProcessSpec{ sampleRate , static_cast<unsigned int>(numSamples), 2 });
            filter2->prepare(juce::dsp::ProcessSpec{ sampleRate , static_cast<unsigned int>(numSamples), 2 });

            upsampled = juce::AudioBuffer<float>(2, numSamples);
            filtered = juce::AudioBuffer<float>(2, numSamples);
            downsampled = juce::AudioBuffer<float>(2, numSamples / 2);

            upsampled.clear();
            filtered.clear();
            downsampled.clear();
        }

        juce::AudioBuffer<float>* oversample(const juce::AudioBuffer<float>& input) {
            //get every other sample
            upsampled.clear();
            for (int i = 0; i < numSamples / 2; i ++) {
                upsampled.setSample(0, i * 2, input.getSample(0, i) * 2);
                upsampled.setSample(1, i * 2, input.getSample(1, i) * 2);
                upsampled.setSample(0, i * 2 + 1, 0);
                upsampled.setSample(1, i * 2 + 1, 0);
            }
            
            auto tmp = juce::dsp::AudioBlock<float>(upsampled);
            auto context = juce::dsp::ProcessContextReplacing<float>(tmp);
            filter1->process(context);

            return &upsampled;
        }

        juce::AudioBuffer<float>* downsample(const juce::AudioBuffer<float>& input) {
            auto tmp = juce::dsp::AudioBlock<float>(filtered);
            auto context = juce::dsp::ProcessContextNonReplacing<float>(juce::dsp::AudioBlock<const float> (input), tmp);

            filter2->process(context);

            //get every other sample
            for (int i = 0; i < numSamples/2; i ++) {
                downsampled.setSample(0, i, filtered.getSample(0, i * 2));
                downsampled.setSample(1, i, filtered.getSample(1, i * 2));
            }       

            return &downsampled;
        }

    };


    void initialize() { // initialize bands and samplers
        if (thread.valid()) {
            try {
                thread.get(); //raa thread safety
            }
            catch (...) {

            }
        }

        //frequency from 20hz to 20000 with possible excess
        jassert(bandWidth > 1); // its a multiplier

        minLowCutoff = 20.0f, maxHighCutoff = sampleRate / nyquist;

        float curFrequency = minLowCutoff;
        float curSampleRate = sampleRate / pow(2.0, numStages-startStage); // ?? maybe?
        int curNumSamples = 1 << startStage;
        while (curFrequency < maxHighCutoff) { // bins are up to 4x sample rate, no point in going further
            float nextFrequency = curFrequency * bandWidth;
            while (curSampleRate / nyquist <= std::min(sampleRate / nyquist, nextFrequency) - eps) {
                curSampleRate *= 2; // 2x the samples
                curNumSamples *= 2;
            }

            //bandsLeft.push_back(band(curNumSamples, curSampleRate, curFrequency, nextFrequency));
            //bandsRight.push_back(band(curNumSamples, curSampleRate, curFrequency, nextFrequency));

            curFrequency = nextFrequency;
        }

        //get the oversamplers / downsamplers i guess
        curSampleRate = sampleRate / (1 << numStages);
        for (int i = 0; i < numStages; i++) {
            curSampleRate *= 2;
            juce::dsp::FIR::Coefficients<float>::Ptr halfband = juce::dsp::FilterDesign<float>::designFIRLowpassHalfBandEquirippleMethod(std::min(0.1f, 8.0f / std::powf(2.0f, static_cast<float>(i))), -60.0f);
            carrierOversamplers.push_back(Oversampler2X(1 << (i + 1), curSampleRate, halfband));
            modulatorOversamplers.push_back(Oversampler2X(1 << (i + 1), curSampleRate, halfband));
        }

        //get buffers
        carrierBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        modulatorBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        processedBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        temporaryBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);

        //get temporary buffers
        carrierBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);
        modulatorBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);
        processedBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);

        //init temp buffers and delays
        stageBuffer.clear();
        stageDelay.clear();
        
        int curDelay = 0;
        for (int i = 0; i <= numStages; i += 1) {
            stageBuffer.push_back(juce::AudioBuffer<float>(2, (1 << i)));
            stageDelay.push_back(IntegerDelay(2, curDelay));

            if (i >= startStage && i < numStages) {
                curDelay += carrierOversamplers[i].delay;
                curDelay *= 2;
            }
        }
        latencyInSamples = curDelay;

        //get filter

    };

    void processPrivate(juce::AudioBuffer<float>& carrier, juce::AudioBuffer<float>& modulator) {

    };

};