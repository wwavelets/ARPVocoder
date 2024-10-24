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


    VocoderProcessor(juce::AudioParameterFloat* attack, juce::AudioParameterFloat* release, juce::AudioParameterFloat* lowcut, juce::AudioParameterFloat* highcut) : attack(attack), release(release), lowcut(lowcut), highcut(highcut) {
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

    std::vector<float>& getEnvelope() {
        for (int i = 0; i < envelope.size(); i++) {
            envelope[i] = (bands[i].followerLeft + bands[i].followerRight) / 2;
        }
        return envelope;
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

                if (carrierBuffer.getMagnitude(0, samplesPerBlock) + processedBuffer.getMagnitude(0, samplesPerBlock) > 0) {
                    thread = std::async(std::launch::async, [&]() {processPrivate(carrierBufferTemp, modulatorBufferTemp);});
                }
                bufferIndex = 0;
            }
            carrier.setSample(0, i, processedBufferTemp.getSample(0, bufferIndex));
            carrier.setSample(1, i, processedBufferTemp.getSample(1, bufferIndex));
        }
    };

private:
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release; // attack and release in milliseconds
    juce::AudioParameterFloat* lowcut;
    juce::AudioParameterFloat* highcut;
    float sampleRate = 48'000, bandWidth = pow(2.0, 1.0 / 12.0);

    const float nyquist = 2.3; // has to be a number greater than 2.0
    const float eps = 0.1; // epsilon

    float minLowCutoff = 20, maxHighCutoff = sampleRate / nyquist;


    int numStages = 11;
    int startStage = 4;
    int samplesPerBlock = (1 << numStages);
    float latencyInSamples = 0;

    const static int filterOrder = 4;


    juce::AudioBuffer<float> carrierBuffer, modulatorBuffer, processedBuffer, temporaryBuffer;
    juce::AudioBuffer<float> carrierBufferTemp, modulatorBufferTemp, processedBufferTemp;
    std::vector<juce::AudioBuffer<float>> stageBuffer;
    std::vector<IntegerDelay> stageDelay;
    std::future<void> thread;
    int bufferIndex = 0;

    using StereoIIRFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    struct band {
        float sampleRate = -1.0f, low = -1.0f, high = -1.0f;
        //std::vector<StereoIIRFilter> carrierFilter, modulatorFilter;

        juce::AudioBuffer<float> carrierOutput, modulatorOutput;
        float followerLeft = 0; // follower = follower * (release-1)/release + newSample * 1/attack
        float followerRight = 0; // follower = follower * (release-1)/release + newSample * 1/attack
        int numSamples = 0;
        //juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> lowpass, highpass;
        StereoIIRFilter carrierBandPass, carrierBandPass2, modulatorBandPass, modulatorBandPass2;

        float gain = 1.0f;

        band(int numSamples, float maxSampleRate, float sampleRate, float low, float high) : numSamples(numSamples), sampleRate(sampleRate), low(low), high(high) {
            //lowpass = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(high, sampleRate, 16);
            //highpass = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(low, sampleRate, 16);

            //carrierFilter.clear();
            //modulatorFilter.clear();
            //for (int _ = 0; _ < 1; _++) {
            //    for (auto& x : lowpass) {
            //        carrierFilter.push_back(StereoIIRFilter());
            //        modulatorFilter.push_back(StereoIIRFilter());
            //        carrierFilter.back().prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            //        modulatorFilter.back().prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            //        *carrierFilter.back().state = *x;
            //        *modulatorFilter.back().state = *x;
            //    }
            //    for (auto& x : highpass) {
            //        carrierFilter.push_back(StereoIIRFilter());
            //        modulatorFilter.push_back(StereoIIRFilter());
            //        carrierFilter.back().prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            //        modulatorFilter.back().prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            //        *carrierFilter.back().state = *x;
            //        *modulatorFilter.back().state = *x;
            //    }
            //}

            carrierOutput = juce::AudioBuffer<float>(2, numSamples);
            modulatorOutput = juce::AudioBuffer<float>(2, numSamples);

            carrierOutput.clear();
            modulatorOutput.clear();

            float freq = sqrt(high * low);

            float bw = 0.5f;
            float w = freq / sampleRate * 3.1415;
            float q = 1.0f / sinh(0.3465736f * bw * w/sin(w)); // bandwidth ykyk
            gain = log10(freq);
            gain *= gain;
            modulatorBandPass.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            *modulatorBandPass.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);
            modulatorBandPass2.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            *modulatorBandPass2.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);
            carrierBandPass.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            *carrierBandPass.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);
            carrierBandPass2.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 2 });
            *carrierBandPass2.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);

        }

        void reset() {
            //for (auto& x : carrierFilter) x.reset();
            //for (auto& x : modulatorFilter) x.reset();
        }


        juce::AudioBuffer<float>* process(const juce::AudioBuffer<float>& carrier, const juce::AudioBuffer<float>& modulator, float attack, float release) {
            //DBG(attack << " " << release);
            carrierOutput.copyFrom(0, 0, carrier, 0, 0, numSamples);
            carrierOutput.copyFrom(1, 0, carrier, 1, 0, numSamples);
            auto carriertmp = juce::dsp::AudioBlock<float>(carrierOutput);
            auto carrierContext = juce::dsp::ProcessContextReplacing<float>(carriertmp);
            modulatorOutput.copyFrom(0, 0, modulator, 0, 0, numSamples);
            modulatorOutput.copyFrom(1, 0, modulator, 1, 0, numSamples);
            auto modulatortmp = juce::dsp::AudioBlock<float>(modulatorOutput);
            auto modulatorContext = juce::dsp::ProcessContextReplacing<float>(modulatortmp);

            //filtering :3

            modulatorBandPass.process(modulatorContext);
            modulatorBandPass2.process(modulatorContext);
            carrierBandPass.process(carrierContext);
            carrierBandPass2.process(carrierContext);

            //attack / release uwuuu
            jassert(carrier.getNumSamples() == modulator.getNumSamples());
            auto carrierArrayLeft = carrierOutput.getWritePointer(0);
            auto modulatorArrayLeft = modulatorOutput.getWritePointer(0);

            float attackCoeff = 1 - exp(-1.0f / attack);
            float releaseCoeff = 1 - exp(-1.0f / release);
            for (int i = 0; i < carrier.getNumSamples(); i++) {
                if (abs(modulatorArrayLeft[i]) > followerLeft)
                    followerLeft += (abs(modulatorArrayLeft[i]) - followerLeft) * attackCoeff;
                else
                    followerLeft += (abs(modulatorArrayLeft[i]) - followerLeft) * releaseCoeff;
                carrierArrayLeft[i] *= gain;
                carrierArrayLeft[i] *= followerLeft;
            }


            auto carrierArrayRight = carrierOutput.getWritePointer(1);
            auto modulatorArrayRight = modulatorOutput.getWritePointer(1);
            for (int i = 0; i < carrier.getNumSamples(); i++) {
                if (abs(modulatorArrayRight[i]) > followerRight)
                    followerRight += (abs(modulatorArrayRight[i]) - followerRight) * attackCoeff;
                else
                    followerRight += (abs(modulatorArrayRight[i]) - followerRight) * releaseCoeff;
                carrierArrayRight[i] *= gain;
                carrierArrayRight[i] *= followerRight;
            }

            return &carrierOutput;
        }
    };
    std::vector<band> bands; // haha it must be stereo
    std::vector<float> envelope;

    struct Oversampler2X { // stereo
        juce::AudioBuffer<float> upsampled, filtered, downsampled;
        int numSamples = 0;
        float sampleRate = 0;
        int delay = 0;

        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> lowpass;
        std::unique_ptr<juce::dsp::Convolution> filter1, filter2;
        Oversampler2X(int numSamples, float sampleRate, juce::dsp::FIR::Coefficients<float>::Ptr& halfband) : numSamples(numSamples), sampleRate(sampleRate) {
            //DBG(numSamples << " " <<  halfband->getFilterOrder());

            int filterSize = halfband->getFilterOrder() + 1;
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
            for (int i = 0; i < numSamples / 2; i++) {
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
            auto context = juce::dsp::ProcessContextNonReplacing<float>(juce::dsp::AudioBlock<const float>(input), tmp);

            filter2->process(context);

            //get every other sample
            for (int i = 0; i < numSamples / 2; i++) {
                downsampled.setSample(0, i, filtered.getSample(0, i * 2));
                downsampled.setSample(1, i, filtered.getSample(1, i * 2));
            }

            return &downsampled;
        }

    };

    std::vector<Oversampler2X> carrierOversamplers, modulatorOversamplers;
    std::vector<StereoIIRFilter> lowBandFilter, highBandFilter;
    std::vector<StereoIIRFilter> lowpassFilter, highpassFilter;

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


        bands.clear();

        minLowCutoff = 20.0f, maxHighCutoff = sampleRate / nyquist;

        float curFrequency = minLowCutoff;
        float curSampleRate = sampleRate / pow(2.0, numStages - startStage); // ?? maybe?
        int curNumSamples = 1 << startStage;
        while (curFrequency < maxHighCutoff) { // bins are up to 4x sample rate, no point in going further
            float nextFrequency = curFrequency * bandWidth;
            while (curSampleRate / nyquist <= std::min(sampleRate / nyquist, nextFrequency) - eps) {
                curSampleRate *= 2; // 2x the samples
                curNumSamples *= 2;
            }

            bands.push_back(band(curNumSamples, sampleRate, curSampleRate, curFrequency, nextFrequency));

            curFrequency = nextFrequency;
        }

        envelope.resize(bands.size(), 0);

        //get the oversamplers / downsamplers i guess
        carrierOversamplers.clear();
        modulatorOversamplers.clear();
        curSampleRate = sampleRate / (1 << numStages);
        for (int i = 0; i < numStages; i++) {
            curSampleRate *= 2;
            juce::dsp::FIR::Coefficients<float>::Ptr halfband = juce::dsp::FilterDesign<float>::designFIRLowpassHalfBandEquirippleMethod(std::min(0.1f, 8.0f / std::powf(2.0f, static_cast<float>(i))), -60.0f);
            carrierOversamplers.push_back(Oversampler2X(1 << (i + 1), curSampleRate, halfband));
            modulatorOversamplers.push_back(Oversampler2X(1 << (i + 1), curSampleRate, halfband));
        }
        jassert(carrierOversamplers.size() == numStages);

        //get buffers
        carrierBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        modulatorBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        processedBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
        temporaryBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);

        //get temporary buffers
        carrierBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);
        modulatorBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);
        processedBufferTemp = juce::AudioBuffer<float>(2, samplesPerBlock);

        carrierBuffer.clear();
        modulatorBuffer.clear();
        processedBuffer.clear();
        temporaryBuffer.clear();
        carrierBufferTemp.clear();
        modulatorBufferTemp.clear();
        processedBufferTemp.clear();

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
        const int filterSize = (filterOrder + 1) / 2 * 2;
        lowBandFilter.resize(filterSize);
        highBandFilter.resize(filterSize);
        lowpassFilter.resize(filterSize);
        highpassFilter.resize(filterSize);
        processedStuff.resize(bands.size());
        for (auto& x : lowpassFilter) {
            x.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(samplesPerBlock), 2 });
        }
        for (auto& x : highpassFilter) {
            x.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(samplesPerBlock), 2 });
        }
        for (auto& x : lowBandFilter) {
            x.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(samplesPerBlock), 2 });
        }
        for (auto& x : highBandFilter) {
            x.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(samplesPerBlock), 2 });
        }
    };


    std::vector<juce::AudioBuffer<float>*> processedStuff;


    void processPrivate(juce::AudioBuffer<float>& carrier, juce::AudioBuffer<float>& modulator) {
        //actual processing
        processedBuffer.clear();

        auto lowpass = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(lowcut->get(), sampleRate, filterOrder); //dont worry its just a small stack allocaiton...
        auto highpass = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(highcut->get(), sampleRate, filterOrder); // uhhhh

        temporaryBuffer.copyFrom(0, 0, carrier, 0, 0, samplesPerBlock);
        temporaryBuffer.copyFrom(1, 0, carrier, 1, 0, samplesPerBlock);
        auto temptmp = juce::dsp::AudioBlock<float>(temporaryBuffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(temptmp);

        for (int i = 0; i < lowpass.size() * 2; i++) {
            *lowpassFilter[i].state = *lowpass[i % static_cast<int>(lowpass.size())];
            lowpassFilter[i].process(context);
        }

        processedBuffer.addFrom(0, 0, temporaryBuffer, 0, 0, samplesPerBlock);
        processedBuffer.addFrom(1, 0, temporaryBuffer, 1, 0, samplesPerBlock);

        temporaryBuffer.copyFrom(0, 0, carrier, 0, 0, samplesPerBlock);
        temporaryBuffer.copyFrom(1, 0, carrier, 1, 0, samplesPerBlock);
        auto context2 = juce::dsp::ProcessContextReplacing<float>(temptmp);

        for (int i = 0; i < highpass.size() * 2; i++) {
            *highpassFilter[i].state = *highpass[i % static_cast<int>(highpass.size())];
            highpassFilter[i].process(context2);
        }

        processedBuffer.addFrom(0, 0, temporaryBuffer, 0, 0, samplesPerBlock);
        processedBuffer.addFrom(1, 0, temporaryBuffer, 1, 0, samplesPerBlock);



        jassert(lowcut->get() < highcut->get());
        auto lowpass2 = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(highcut->get(), sampleRate, filterOrder); //dont worry its just a small stack allocaiton...
        auto highpass2 = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(lowcut->get(), sampleRate, filterOrder); // uhhhh

        auto carriertmp = juce::dsp::AudioBlock<float>(carrier);
        auto context3 = juce::dsp::ProcessContextReplacing<float>(carriertmp);

        for (int i = 0; i < lowpass2.size() * 2; i++) { // im assuming each iircoefficients is 5, and that each filter order = # of coefifcients?
            *lowBandFilter[i].state = *lowpass2[i % static_cast<int>(lowpass.size())];
            lowBandFilter[i].process(context3);
        }

        for (int i = 0; i < highpass2.size() * 2; i++) { // im assuming each iircoefficients is 5, and that each filter order = # of coefifcients?
            *highBandFilter[i].state = *highpass2[i % static_cast<int>(highpass.size())];
            highBandFilter[i].process(context3);
        }

        auto* currentCarrier = &carrier, * currentModulator = &modulator;
        int currentBand = static_cast<int>(bands.size()) - 1; // two pointers!
        float curSampleRate = sampleRate;

        for (int stage = numStages; stage >= 0; stage--) { // haha its a bit more stages then numstages would imply.. numstages is just the shits
            if (stage < numStages) {
                curSampleRate /= 2;
                currentCarrier = carrierOversamplers[stage].downsample(*currentCarrier);
                currentModulator = modulatorOversamplers[stage].downsample(*currentModulator);
            }

            float attackSamples = attack->get() * curSampleRate;
            float releaseSamples = release->get() * curSampleRate;
            while (currentBand >= 0 && bands[currentBand].sampleRate + eps > curSampleRate) {
                if (lowcut->get() <= bands[currentBand].high && bands[currentBand].low <= highcut->get()) {
                    //process
                    processedStuff[currentBand] = bands[currentBand].process(*currentCarrier, *currentModulator, attackSamples, releaseSamples);
                }
                else {
                    bands[currentBand].reset(); // reset.
                }
                currentBand -= 1;
            }
        }

        currentBand = 0;
        juce::AudioBuffer<float> tmp(2, 1); //small stack allocation surely
        currentCarrier = &tmp;
        currentCarrier->clear();
        for (int stage = 0; stage <= numStages; stage++) {
            int curNumSamples = (1 << stage);
            stageBuffer[stage].clear();
            while (currentBand < static_cast<int>(bands.size()) && bands[currentBand].sampleRate < curSampleRate + eps) {
                if (lowcut->get() <= bands[currentBand].high && bands[currentBand].low <= highcut->get()) {
                    stageBuffer[stage].addFrom(0, 0, *processedStuff[currentBand], 0, 0, curNumSamples);
                    stageBuffer[stage].addFrom(1, 0, *processedStuff[currentBand], 1, 0, curNumSamples);
                }
                currentBand += 1;
            }

            if (stage == numStages) {
                //stageBuffer[stage].addFrom(0, 0, processedBuffer, 0, 0, samplesPerBlock);
                //stageBuffer[stage].addFrom(1, 0, processedBuffer, 1, 0, samplesPerBlock);
            }
            stageDelay[stage].process(stageBuffer[stage]);

            currentCarrier->addFrom(0, 0, stageBuffer[stage], 0, 0, curNumSamples);
            currentCarrier->addFrom(1, 0, stageBuffer[stage], 1, 0, curNumSamples);

            if (stage < numStages) {
                currentCarrier = carrierOversamplers[stage].oversample(*currentCarrier);
                curSampleRate *= 2;
            }
        }


        processedBuffer.copyFrom(0, 0, *currentCarrier, 0, 0, samplesPerBlock);
        processedBuffer.copyFrom(1, 0, *currentCarrier, 1, 0, samplesPerBlock);
    };

};