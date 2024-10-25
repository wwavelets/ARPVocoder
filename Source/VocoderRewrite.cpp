/*
  ==============================================================================

    Rewrite.h
    Created: 11 Jun 2024 12:07:69pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "IntegerDelay.h"
#include "Interface/LineEditor.h"


struct VocoderBandData {
    float gain = 0.5f; //initial gain before processing
    float modulatorGain = 0.5f; //modulator gain before processing
    float postGain = 0.5f;
    float delay = 0.0f; // meoww
    
    float envelope = 0; // for visuals
};

struct VocoderParameters {
    juce::AudioParameterFloat* mix = nullptr;
    juce::AudioParameterFloat* attack = nullptr;
    juce::AudioParameterFloat* release = nullptr;
    juce::AudioParameterFloat* carrierBandWidth = nullptr;
    juce::AudioParameterFloat* modulatorBandWidth = nullptr;
    juce::AudioParameterFloat* formant = nullptr; // formant shift from -12 to 12
    std::vector<VocoderBandData>* bandData = nullptr;
    LineEditor* carrierMapping = nullptr;
    LineEditor* modulatorMapping = nullptr;
};

class VocoderProcessor {
public:


    VocoderProcessor(VocoderParameters parameters, juce::AudioProcessor* head) : parameters(parameters), head(head){
        initialize();
        carrierLatencyCompensation.prepare(juce::dsp::ProcessSpec{ sampleRate, 1, 2 });
        modulatorLatencyCompensation.prepare(juce::dsp::ProcessSpec{ sampleRate, 1, 2 });
    };
    ~VocoderProcessor() {

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
        std::fill(gain.begin(), gain.end(), 0);
        if (mode == Default) {
            processorLeft.load(std::memory_order_acquire)->getEnvelope(gain);
            processorRight.load(std::memory_order_acquire)->getEnvelope(gain);
        }
        else if(mode == LinearPhase) {
            if (processorLeftFIR.load(std::memory_order_acquire) != nullptr && processorRightFIR.load(std::memory_order_acquire) != nullptr) {
                processorLeftFIR.load(std::memory_order_acquire)->getEnvelope(gain);
                processorRightFIR.load(std::memory_order_acquire)->getEnvelope(gain);
            }
        }
        return gain;
    };
    int getNumBands() {
        if (mode == Default) {
            return processorLeft.load(std::memory_order_acquire)->getNumBands();
        }
        else if (mode == LinearPhase) {
            return processorLeftFIR.load(std::memory_order_acquire)->getNumBands();
        }
    }

    double getSampleRate() {
        return sampleRate;
    }
    double getMinFreq() {
        return 20.0152312641;
    }

    

    void process(juce::AudioBuffer<float>& carrier, const juce::AudioBuffer<float>& modulator) {
        auto carrierLeft = carrier.getWritePointer(0);
        auto carrierRight = carrier.getWritePointer(1);
        auto modulatorLeft = modulator.getReadPointer(0);
        auto modulatorRight = modulator.getReadPointer(1);
        int numSamples = carrier.getNumSamples();

        if (mode == Default) {
            if (curSample >= bufferSize) curSample = 0;
            for (int i = 0; i < numSamples; i++) {
                const float scalar = log(static_cast<float>(getNumBands())) / log(2.0f) * 2.0f; // should scale up based on number of bands
                carrierBufferLeft[curSample] = carrierLeft[i]* scalar;
                carrierBufferRight[curSample] = carrierRight[i]* scalar;
                modulatorBufferLeft[curSample] = modulatorLeft[i]* scalar;
                modulatorBufferRight[curSample] = modulatorRight[i]* scalar;
                
                {
                    carrierLatencyCompensation.pushSample(0, carrierLeft[i]);
                    carrierLatencyCompensation.pushSample(1, carrierRight[i]);
                    carrierLeft[i] = carrierLatencyCompensation.popSample(0);
                    carrierRight[i] = carrierLatencyCompensation.popSample(1);
                    modulatorLatencyCompensation.pushSample(0, modulatorLeft[i]);
                    modulatorLatencyCompensation.pushSample(1, modulatorRight[i]);

                    if ((*parameters.mix) < 0) {
                        carrierLeft[i] = modulatorLatencyCompensation.popSample(0);
                        carrierRight[i] = modulatorLatencyCompensation.popSample(1);
                    }
                    else {
                        modulatorLatencyCompensation.popSample(0);
                        modulatorLatencyCompensation.popSample(1);
                    }
                }

                if (resLeft != nullptr)
                    carrierLeft[i] = carrierLeft[i] * abs(*parameters.mix) + resLeft[curSample] / scalar * (1- abs(*parameters.mix));
                if (resRight != nullptr)
                    carrierRight[i] = carrierRight[i] * abs(*parameters.mix) + resRight[curSample] / scalar * (1 - abs(*parameters.mix));

                curSample++;
                if (curSample == bufferSize) {
                    processorLeft.load(std::memory_order_acquire)->updateParameters();
                    processorRight.load(std::memory_order_acquire)->updateParameters();
                    curSample = 0;

                    //filter modulatorBufferLeft, modulatorBufferRight??
                    resLeft = processorLeft.load(std::memory_order_acquire)->process(carrierBufferLeft, modulatorBufferLeft);
                    resRight = processorRight.load(std::memory_order_acquire)->process(carrierBufferRight, modulatorBufferRight);

                }
            }
        }
        else if(mode == LinearPhase) {
            if (curSample >= bufferSizeFIR) curSample = 0;
            if (curSampleFIR >= bufferSize2) curSampleFIR = 0;
            for (int i = 0; i < numSamples; i++) {
                carrierBufferLeft[curSampleFIR] = carrierLeft[i];
                carrierBufferRight[curSampleFIR] = carrierRight[i];
                modulatorBufferLeft[curSampleFIR] = modulatorLeft[i];
                modulatorBufferRight[curSampleFIR] = modulatorRight[i];

                {
                    carrierLatencyCompensation.pushSample(0, carrierLeft[i]);
                    carrierLatencyCompensation.pushSample(1, carrierRight[i]);
                    carrierLeft[i] = carrierLatencyCompensation.popSample(0);
                    carrierRight[i] = carrierLatencyCompensation.popSample(1);
                    modulatorLatencyCompensation.pushSample(0, modulatorLeft[i]);
                    modulatorLatencyCompensation.pushSample(1, modulatorRight[i]);

                    if ((*parameters.mix) < 0) {
                        carrierLeft[i] = modulatorLatencyCompensation.popSample(0);
                        carrierRight[i] = modulatorLatencyCompensation.popSample(1);
                    }
                    else {
                        modulatorLatencyCompensation.popSample(0);
                        modulatorLatencyCompensation.popSample(1);
                    }
                }

                carrierLeft[i] = carrierLeft[i] * abs(*parameters.mix) + resLeftFIR[curSample] * (1 - abs(*parameters.mix)); resLeftFIR[curSample] = 0;
                carrierRight[i] = carrierRight[i] * abs(*parameters.mix) + resRightFIR[curSample] * (1 - abs(*parameters.mix)); resRightFIR[curSample] = 0;

                curSample++;
                curSampleFIR++;

                if (curSample == bufferSizeFIR) {
                    curSample = 0;
                }
                if (curSampleFIR == bufferSize2) {
                    curSampleFIR -= bufferSize2 / overlap;
                    if (processorLeftFIR.load(std::memory_order_acquire) == nullptr) continue;
                    if (processorRightFIR.load(std::memory_order_acquire) == nullptr) continue;
                    resLeft = processorLeftFIR.load(std::memory_order_acquire)->process(carrierBufferLeft, modulatorBufferLeft);
                    resRight = processorRightFIR.load(std::memory_order_acquire)->process(carrierBufferRight, modulatorBufferRight);

                    for (int i = 0; i < bufferSizeFIR; i++) {
                        int t = curSample + i; if (t >= bufferSizeFIR) t -= bufferSizeFIR;
                        resLeftFIR[t] += resLeft[i];
                        resRightFIR[t] += resRight[i];
                    }

                    for (int i = 0; i + bufferSize2 / overlap < bufferSize2; i++) {
                        carrierBufferLeft[i] = carrierBufferLeft[i + bufferSize2 / overlap];
                        carrierBufferRight[i] = carrierBufferRight[i + bufferSize2 / overlap];
                        modulatorBufferLeft[i] = modulatorBufferLeft[i + bufferSize2 / overlap];
                        modulatorBufferRight[i] = modulatorBufferRight[i + bufferSize2 / overlap];
                    }
                }
            }
        }
    };


    void setFIR() { //set mode to fir
        mode = LinearPhase;
        updateLatency();
    }

    void setIIR() { //set mode to iir
        mode = Default;
        updateLatency();
    }

private:
    VocoderParameters parameters;

    double sampleRate = 48'000.0;
    constexpr static double maxFreq = 20'000.0; // anything above this isn't audible
    float bandWidth = 2.0;// pow(2.0, 1.0 / 12.0);//pow(2.0, 1.0 / 12.0);

    constexpr static float nyquist = 2.5f;//2.3f*2.0f; // has to be a number greater than 2.0
    constexpr static float nyquist2 = 2.05f;
    constexpr static float eps = 0.1f; // epsilon
    int curSample = 0;
    int curSampleFIR = 0;

    float minLowCutoff = 20, maxHighCutoff = sampleRate / nyquist;


    int numStages = 7;
    float latencyInSamples = 0.0f;

    static constexpr int bufferSize = 512;
    static constexpr int bufferSize2 = 408;
    static constexpr int filterSize = 512;
    static constexpr int overlap = 4;
    static constexpr int bufferSizeFIR = bufferSize + filterSize + bufferSize2 / overlap;
    float carrierBufferLeft[bufferSize];
    float carrierBufferRight[bufferSize];
    float modulatorBufferLeft[bufferSize];
    float modulatorBufferRight[bufferSize];
    float resLeftFIR[bufferSizeFIR]{};
    float resRightFIR[bufferSizeFIR]{};

    class FIRVocoderInternal {
    public:
        VocoderParameters parameters;
        int numBands = 0;
        static constexpr int order = 10; // 1024
        static constexpr int fftSize = 1 << order;
        double sampleRate = 0.0f;
        FIRVocoderInternal(double sampleRate, int numBands, VocoderParameters parameters) : sampleRate(sampleRate), parameters(parameters), numBands(numBands) {
            fft = std::make_unique<juce::dsp::FFT>(order);
            carrierCopy = std::make_shared<float[]>(fftSize *2); //2x get size for some reason
            modulatorCopy = std::make_shared<float[]>(fftSize *2); //2x get size for some reason
            window = std::make_shared<float[]>(fftSize);
            std::fill(window.get(), window.get() + fftSize, 0.0f);
            for (int i = 0; i < bufferSize2; i++) {
                window[i] = 0.5f * cos(2.0f*M_PI* static_cast<float>(i+0.5f) / bufferSize2) + 0.5f;
                //DBG(window[i]<<",");
            }
            //DBG("\n");

            filteredCarrier = std::make_unique<std::unique_ptr<float[]>[]>(numBands);
            filteredModulator = std::make_unique<std::unique_ptr<float[]>[]>(numBands);
            for (int i = 0; i < numBands; i++) {
                filteredCarrier[i] = std::make_unique<float[]>(fftSize *2);
                filteredModulator[i] = std::make_unique<float[]>(fftSize *2);
            }

            bandFilters = std::make_unique<std::unique_ptr<float[]>[]>(numBands);
            filterL = std::make_unique<int[]>(numBands);
            filterR = std::make_unique<int[]>(numBands);
            lnFilterFreq = std::make_unique<float[]>(numBands);

            //filters
            float prevFreq = 20.0152312641f;
            float curFreq =  std::max(90.0f, 20.0152312641f *powf(maxFreq/ 20.0152312641f, 1.0f/numBands));
            float midFreq = sqrt(prevFreq * curFreq);
            lnFilterFreq[0] = log(midFreq);

            //first filter
            auto flt = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod(curFreq, sampleRate, filterSize, juce::dsp::WindowingFunction<float>::hamming);
            auto coefficients = flt.get()->getRawCoefficients();
            bandFilters[0] = std::make_unique<float[]>(fftSize * 2);
            std::fill(&bandFilters[0][0], &bandFilters[0][fftSize], 0);
            std::copy(coefficients, coefficients+ filterSize +1, &bandFilters[0][0]);

            // mid filters
            for (int i = 1; i + 1 < numBands; i++) {
                prevFreq = curFreq;
                curFreq = std::max(curFreq + 120.0f, curFreq * powf(maxFreq / curFreq, 1.0f / (numBands-i)));
                midFreq = sqrt(curFreq * prevFreq);
                lnFilterFreq[i] = log(midFreq);
                //filter stuff
                auto flt = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod(curFreq, sampleRate, filterSize, juce::dsp::WindowingFunction<float>::hamming);
                auto coefficients = flt.get()->getRawCoefficients();
                bandFilters[i] = std::make_unique<float[]>(fftSize * 2);
                std::fill(&bandFilters[i][0], &bandFilters[i][fftSize], 0);
                std::copy(coefficients, coefficients + filterSize + 1, &bandFilters[i][0]);
            }

            // last filter
            prevFreq = curFreq;
            curFreq = maxFreq;
            midFreq = sqrt(curFreq * prevFreq);
            lnFilterFreq[numBands - 1] = log(midFreq);
            bandFilters[numBands - 1] = std::make_unique<float[]>(fftSize * 2);
            std::fill(&bandFilters[numBands - 1][0], &bandFilters[numBands - 1][fftSize], 0);
            bandFilters[numBands - 1][filterSize / 2] = 1.0f;

            for (int i = numBands-1; i > 0; i--) {
                std::transform(&bandFilters[i][0], &bandFilters[i][fftSize], &bandFilters[i - 1][0], &bandFilters[i][0], std::minus<float>());
            }

            constexpr float eps = 5e-4;
            for (int i = 0; i < numBands; i++) {
                fft->performRealOnlyForwardTransform(&bandFilters[i][0]);

                filterL[i] = 0; filterR[i] = fftSize-1;
                while (filterL[i]+1 < fftSize&& abs(bandFilters[i][filterL[i]]) < eps) {
                    filterL[i]++;
                }
                filterL[i] -= (filterL[i] & 1);

                while (filterR[i] > 0 && abs(bandFilters[i][filterR[i]]) < eps) {
                    filterR[i]--;
                }
                filterR[i]++; // make it exclusive
                filterR[i] += (filterR[i] & 1); //it really should be even

                //DBG("Filter bounds: " << filterL[i] << "," << filterR[i] << "\n");
            }


            //envelope followers
            carrierFollower = std::make_unique<float[]>(numBands);
            std::fill(&carrierFollower[0], &carrierFollower[numBands], 0.0f);
            follower = std::make_unique<float[]>(numBands);
            std::fill(&follower[0], &follower[numBands], 0.0f);
            realFollower = std::make_unique<float[]>(numBands);
            std::fill(&realFollower[0], &realFollower[numBands], 0.0f);

            //delay lines
            delayLines = std::make_unique<std::unique_ptr<delayLine>[]>(numBands);
            for (int i = 0; i < numBands; i++) {
                delayLines[i] = std::make_unique<delayLine>(sampleRate * 2);
            }
        }

        std::shared_ptr<float[]> process(float* _carrier, float* _modulator) {
            const float attackCoeff = 1 - exp(-1.0f / (sampleRate / (bufferSize2 /overlap) * parameters.attack->get()));
            const float releaseCoeff = 1 - exp(-1.0f / (sampleRate / (bufferSize2 / overlap) * parameters.release->get()));

            //DBG("MROWWW " << attackCoeff << " " << releaseCoeff);


            float* carrier = carrierCopy.get(); float* modulator = modulatorCopy.get();
            std::fill(carrier, carrier + fftSize, 0.0f);
            std::fill(modulator, modulator + fftSize, 0.0f);
            std::copy(_carrier, _carrier + bufferSize2, carrier);
            std::copy(_modulator, _modulator + bufferSize2, modulator);
            //multiply with window

            std::transform(carrier, carrier + fftSize, &window[0], carrier, std::multiplies<float>());
            std::transform(modulator, modulator + fftSize, &window[0], modulator, std::multiplies<float>());

            fft->performRealOnlyForwardTransform(carrier);
            fft->performRealOnlyForwardTransform(modulator);


            for (int i = 0; i < numBands; i++) {
                //complex multiplication is annoying
                for (int j = filterL[i]; j < filterR[i]; j += 2) {
                    filteredCarrier[i][j] = carrier[j] * bandFilters[i][j] - carrier[j + 1] * bandFilters[i][j + 1];
                    filteredCarrier[i][j + 1] = carrier[j] * bandFilters[i][j + 1] + carrier[j + 1] * bandFilters[i][j];
                }


                for (int j = filterL[i]; j < filterR[i]; j += 2) {
                    filteredModulator[i][j] = modulator[j] * bandFilters[i][j] - modulator[j + 1] * bandFilters[i][j + 1];
                    filteredModulator[i][j + 1] = modulator[j] * bandFilters[i][j + 1] + modulator[j + 1] * bandFilters[i][j];
                }
                //lmao
                float scale = sqrt(std::accumulate(&bandFilters[i][filterL[i]], &bandFilters[i][filterR[i]], 0.0f, [](const float& a, const float& b) {return a + b * b; }));
                float carrierRMS = sqrt(std::accumulate(&filteredCarrier[i][filterL[i]], &filteredCarrier[i][filterR[i]], 0.0f, [](const float& a, const float& b) {return a + b*b/fftSize; }))/scale;
                float modulatorRMS = sqrt(std::accumulate(&filteredModulator[i][filterL[i]], &filteredModulator[i][filterR[i]], 0.0f, [](const float& a, const float& b) {return a + b*b / fftSize; })) / scale;

                
                carrierRMS *= powf(1.414, (lnFilterFreq[i]-lnFilterFreq[0]) / logf(2.0f)); // 3db per octave 
                modulatorRMS *= powf(1.414, (lnFilterFreq[i] - lnFilterFreq[0]) / logf(2.0f));

                carrierRMS *= (*parameters.bandData)[i].gain;
                modulatorRMS *= (*parameters.bandData)[i].modulatorGain;

                if (carrierRMS > carrierFollower[i])
                    carrierFollower[i] += (carrierRMS - carrierFollower[i]) * attackCoeff;
                else
                    carrierFollower[i] += (carrierRMS - carrierFollower[i]) * releaseCoeff;

                if (modulatorRMS > follower[i])
                    follower[i] += (modulatorRMS - follower[i]) * attackCoeff;
                else
                    follower[i] += (modulatorRMS - follower[i]) * releaseCoeff;

            }

            for (int i = 0; i < numBands; i++) {


                float gain = (*parameters.bandData)[i].gain;
                if (carrierFollower[i] > 0.001) gain *= parameters.carrierMapping->query(carrierFollower[i]) / carrierFollower[i];

                float myFreq = lnFilterFreq[i] - log(powf(2.0f, parameters.formant->get() / 12.0f));

                float lerpFollower = 0.0f;

                if (myFreq < log(20.0152312641f) || myFreq > log(maxFreq)) {
                    lerpFollower = 0.0f;
                }
                else for (int j = 0; j <= numBands; j++) {
                    if (j == 0) {
                        if (myFreq < lnFilterFreq[j]) {
                            lerpFollower = std::lerp(0.0, follower[j], (myFreq - log(20.01523126415)) / (lnFilterFreq[j] - log(20.01523126415)));
                            break;
                        }
                    }
                    else if (j == numBands) {
                        if (myFreq > lnFilterFreq[j]) {
                            lerpFollower = std::lerp(follower[j-1], 0.0, (myFreq - lnFilterFreq[j - 1]) / (log(maxFreq) - lnFilterFreq[j - 1]));
                            break;
                        }
                    }
                    else {
                        //lerp between j, j-1
                        if (lnFilterFreq[j-1] <= myFreq && myFreq <= lnFilterFreq[j]) {
                            lerpFollower = std::lerp(follower[j - 1], follower[j], (myFreq - lnFilterFreq[j - 1]) / (lnFilterFreq[j] - lnFilterFreq[j - 1]));
                            break;
                        }
                    }
                }
                realFollower[i] = lerpFollower;
                gain *= parameters.modulatorMapping->query(lerpFollower);


                gain *= (*parameters.bandData)[i].postGain;

                std::for_each(&filteredCarrier[i][filterL[i]], &filteredCarrier[i][filterR[i]], [gain](float& x) {x *= gain; });

                delayLines[i]->setDelay(static_cast<int>(std::powf((*parameters.bandData)[i].delay, 3) * sampleRate*2.0f + 0.5f));
                delayLines[i]->process(filterL[i], filterR[i], filteredCarrier[i]);
            }

            std::fill(carrier, carrier + fftSize, 0.0f);

            for (int i = 0; i < numBands; i++) {
                for (int j = filterL[i]; j < filterR[i]; j++)carrier[j] += filteredCarrier[i][j];
            }

            fft->performRealOnlyInverseTransform(carrier);
            return carrierCopy;
        }
        int getEnvelope(std::vector<float>& gain) {
            int cur = 0;
            for (int id = 0; id < numBands; id++) {
                gain[cur++] += getGain(id); //+= because we are summing left and right
            }
            return cur;
        }
        int getNumBands() {
            return numBands;
        }
    private:
        struct delayLine {
            std::unique_ptr<std::unique_ptr<float[]>[]> buffer;
            constexpr static int delta = bufferSize / overlap;
            int numBuffer = 0, numMod = 0, curBuffer = 0;
            delayLine(int maxDelayInSamples) {
                int num = maxDelayInSamples/delta;
                buffer = std::make_unique<std::unique_ptr<float[]>[]>(num);
                for (int i = 0; i < num; i++) {
                    buffer[i] = std::make_unique<float[]>(fftSize*2);
                    std::fill(&buffer[i][0], &buffer[i][fftSize*2], 0.0f);
                }

            }
            void setDelay(int delayInSamples) {
                numBuffer = delayInSamples / delta;
                numMod = delayInSamples % delta;
            }
            void process(int l, int r, std::unique_ptr<float[]>& input) {
                std::swap(buffer[curBuffer++], input);
                if (curBuffer >= numBuffer) curBuffer = 0;
                for (int i = l; i < r; i+=2) {
                    int freq = i / 2;
                    //e^(-2 pi freq delay / n)
                    float theta = -2.0f * M_PI * static_cast<float>(freq) * static_cast<float>(numMod) / static_cast<float>(fftSize);
                    float a = input[i];
                    float b = input[i+1];
                    float c = cos(theta);
                    float d = sin(theta);
                    input[i] = a * c - b * d;
                    input[i + 1] = a * d + b * c;
                }
            }
        };
        float getGain(int id) {
            float gain = (*parameters.bandData)[id].gain;
            if (carrierFollower[id] > 0.001) gain *= parameters.carrierMapping->query(carrierFollower[id]) / carrierFollower[id];
            gain *= parameters.modulatorMapping->query(realFollower[id]);
            gain *= (*parameters.bandData)[id].postGain;
            return  gain;
        }
        std::unique_ptr<std::unique_ptr<delayLine>[]> delayLines;
        std::unique_ptr<juce::dsp::FFT> fft = nullptr;
        std::shared_ptr<float[]> carrierCopy, modulatorCopy, window;
        std::unique_ptr<std::unique_ptr<float[]>[]> filteredCarrier, filteredModulator, bandFilters;
        std::unique_ptr<float[]> carrierFollower, follower, realFollower;
        std::unique_ptr<int[]> filterL, filterR;
        std::unique_ptr<float[]> lnFilterFreq;
    };

    struct band {
        float follower = 0.0f, carrierFollower = 0.0f;

        constexpr static int order = 3;
        juce::dsp::IIR::Filter<float> bandpass[2][order]; //carrier, modulator
        float sampleRate = -1.0f, low = -1.0f, high = -1.0f;

        float gain = 1.0f;
        float attackCoeff = 0;
        float releaseCoeff = 0;

        int numSamples = 0;

        VocoderParameters parameters;
        int id = 0;

        std::shared_ptr<float[]> carrierCopy, modulatorCopy; // i dont want to overwrite
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay{ 48000 * 2 };
        std::unique_ptr<float[]> followerVals;
        float realFollower = 0.0f;
        std::shared_ptr<std::vector<band*>> bands;
        band(int numSamples, float sampleRate, float low, float high, VocoderParameters parameters, int id, std::shared_ptr<std::vector<band*>> bands) : numSamples(numSamples), sampleRate(sampleRate),
            low(low), high(high), parameters(parameters), id(id), bands(bands) {
            for (int type = 0; type < 2; type++) {
                for (int i = 0; i < order; i++) {
                    bandpass[type][i].prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 1 });
                }
            }


            for (int type = 0; type < 2; type++) {
                for (int i = 0; i < order; i++) {
                    bandpass[type][i].reset();
                }
            }

            carrierCopy = std::make_shared<float[]>(numSamples);
            modulatorCopy = std::make_shared<float[]>(numSamples);
            followerVals = std::make_unique<float[]>(numSamples);

            delay.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0f + 0.5f));
            juce::dsp::ProcessSpec spec{ sampleRate, 512, 1 }; // Set sample rate, block size, and num channels
            delay.prepare(spec);
            delay.reset();
            updateParameters();
        }

        void pointerStuff() {
            bands->push_back(this);
            jassert((*bands)[id] == this);
            //store myself into the bands list
        }
        void updateParameters() {
            if ((*bands).size() > id) {
                jassert((*bands)[id] == this);
            }
            attackCoeff = 1 - exp(-1.0f / (sampleRate * parameters.attack->get()));
            releaseCoeff = 1 - exp(-1.0f / (sampleRate * parameters.release->get()));


            float freq = std::min(sqrt(high * low), sampleRate / 2.0f);
            float bw = log2(high / low) * parameters.carrierBandWidth->get();
            float w = freq / sampleRate * 3.1415;
            float q = 0.5f / sinh(0.3465736 * bw * w / sin(w)); // 0.34... = ln(2)/2. estimated formula for bandwidth


            for (int i = 0; i < order; i++) {
                bandpass[0][i].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);
            }

            bw = log2(high / low) * parameters.modulatorBandWidth->get();
            q = 0.5f / sinh(0.3465736 * bw * w / sin(w));


            for (int i = 0; i < order; i++) {
                bandpass[1][i].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, q);
            }
        }

        void processModulator(const float* modulator) {
            auto modulatorCopyPtr = modulatorCopy.get();
            memcpy(modulatorCopyPtr, modulator, sizeof(float) * numSamples);
            auto modulatortmp = juce::dsp::AudioBlock<float>(&modulatorCopyPtr, 1, numSamples);
            bandpass[1][0].process(juce::dsp::ProcessContextReplacing<float>(modulatortmp));
            bandpass[1][1].process(juce::dsp::ProcessContextReplacing<float>(modulatortmp));


            float modulatorGain = (*parameters.bandData)[id].modulatorGain;
            modulatorGain = 4 * modulatorGain * modulatorGain; //idek

            for (int i = 0; i < numSamples; i++) {
                modulatorCopy[i] *= modulatorGain;

                if (abs(modulatorCopy[i]) > follower)
                    follower += (abs(modulatorCopy[i]) - follower) * attackCoeff;
                else
                    follower += (abs(modulatorCopy[i]) - follower) * releaseCoeff;
                followerVals[i] = follower;
            }
        }

        float queryFollower(int id) { // id/bufferSize
            //id = x * bufferSize/numSamples;
            int l = id * numSamples / bufferSize;
            int r = l + 1; // i guess
            float pos = static_cast<float>(id) * static_cast<float>(numSamples) / static_cast<float>(bufferSize);
            float res = followerVals[l] * (1 - (pos - l));
            if (r < numSamples) res += followerVals[r] * (pos - l);
            return res;
        }

        __forceinline float* process(const float* carrier) {
            auto carrierCopyPtr = carrierCopy.get();

            memcpy(carrierCopyPtr, carrier, sizeof(float) * numSamples);

            auto carriertmp = juce::dsp::AudioBlock<float>(&carrierCopyPtr, 1, numSamples);
            bandpass[0][0].process(juce::dsp::ProcessContextReplacing<float>(carriertmp));
            bandpass[0][1].process(juce::dsp::ProcessContextReplacing<float>(carriertmp));


            float carrierGain = (*parameters.bandData)[id].gain;
            carrierGain = 4 * carrierGain * carrierGain; // scaling


            float postGain = (*parameters.bandData)[id].postGain;
            postGain = 4 * postGain * postGain; // scaling

            //frequency after formant shifting
            float freq = id - log(powf(2.0f, (1.0f / 12.0f)*parameters.formant->get())) /log(high/low);
            int l = static_cast<int>(freq);
            int r = l + 1;
            float lerpval = freq - static_cast<int>(l);


            for (int i = 0; i < numSamples; i++) {
                carrierCopy[i] *= carrierGain;

                if (abs(carrierCopy[i]) > carrierFollower)
                    carrierFollower += (abs(carrierCopy[i]) - carrierFollower) * attackCoeff;
                else
                    carrierFollower += (abs(carrierCopy[i]) - carrierFollower) * releaseCoeff;


                if (carrierFollower > 0.001) carrierCopy[i] *= parameters.carrierMapping->query(carrierFollower) / carrierFollower;

                float res = 0.0f;
                if (l >= 0 && l < bands->size()) res += (1.0 - lerpval) * (*bands)[l]->queryFollower(i);
                if (r >= 0 && r < bands->size()) res += (lerpval) * (*bands)[r]->queryFollower(i);
                carrierCopy[i] *= parameters.modulatorMapping->query(res); // TODO: FIX FOR FORMANTS
                carrierCopy[i] *= postGain;
            }

            delay.setDelay(powf((*parameters.bandData)[id].delay, 3.0f) * 2.0f * sampleRate);
            delay.process(juce::dsp::ProcessContextReplacing<float>(carriertmp));
            return carrierCopy.get();
        }

        float getGain() {
            float gain = (*parameters.bandData)[id].gain;
            if (carrierFollower > 0.001) gain *= parameters.carrierMapping->query(carrierFollower) / carrierFollower;


            float freq = id - log(powf(2.0f, (1.0f / 12.0f) * parameters.formant->get())) / log(high / low);
            int l = static_cast<int>(freq);
            int r = l + 1;
            float lerpval = freq - static_cast<int>(l);
            float res = 0.0f;
            if (l >= 0 && l < bands->size()) res += (1.0 - lerpval) * (*bands)[l]->queryFollower(0);
            if (r >= 0 && r < bands->size()) res += (lerpval) * (*bands)[r]->queryFollower(0);

            gain *= parameters.modulatorMapping->query(res);
            gain *= (*parameters.bandData)[id].postGain;
            return  gain;
        }
    };

    class VocoderInternal {
    public:
        VocoderInternal(int numSamples, int numStages, double sampleRate, double startFreq, double endFreq, float bandWidth, VocoderParameters parameters, std::shared_ptr<std::vector<band*>> allBands) : numSamples(numSamples) {
            //design halfband filter

            auto lowpass = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderEllipticMethod(1.0 / 2.0 / VocoderProcessor::nyquist2, 1.0, 0.1, -0.1, -60);
            for (int i = 0; i < 3; i++) {
                for (auto& x : lowpass) {
                    halfband[i].push_back(juce::dsp::IIR::Filter<float>(x));
                    halfband[i].back().prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<unsigned int>(numSamples), 1 });
                    halfband[i].back().reset();
                }
            }

            //get child & bands
            if (numStages > 1) {
                childProcessor = std::make_shared<VocoderInternal>(numSamples/2, numStages - 1, sampleRate / 2.0f, startFreq, std::min(endFreq, sampleRate / 2.0f / VocoderProcessor::nyquist), bandWidth, parameters, allBands);
                startFreq = childProcessor->resFreq; // raa
                numBands = childProcessor->numBands;
            }

            resFreq = startFreq;
            int cur = 0;
            if(childProcessor != nullptr) cur = childProcessor->getNumBands();
            
            for (double freq = startFreq; freq * bandWidth < endFreq; freq *= bandWidth) {
                jassert(freq > 0);
                double nextFreq = freq * bandWidth;

                //create the band
                bands.push_back(band(numSamples, sampleRate, freq, nextFreq, parameters, cur++, allBands));
                resFreq = nextFreq;
                numBands++;
            }

            for (auto& band : bands) band.pointerStuff();
            //init parameters
            updateParameters();

            //allocate memory
            res = std::make_shared<float[]>(numSamples);
            downsampledCarrier = std::make_shared<float[]>(numSamples/2);
            downsampledModulator = std::make_shared<float[]>(numSamples/2);
            carrierCopy = std::make_unique<float[]>(numSamples);
        }
        void updateParameters() {
            for (auto& x : bands) x.updateParameters();
            if (childProcessor != nullptr) {
                childProcessor->updateParameters();
            }
        }
        int getNumBands() {
            return numBands;
        }
        int getEnvelope(std::vector<float>& gain) {
            int cur = 0;
            if (childProcessor != nullptr) {
                cur = childProcessor->getEnvelope(gain);
            }
            for (auto& x : bands) {
                gain[cur++] += x.getGain(); //+= because we are summing left and right
            }
            return cur;
        }

        __forceinline std::shared_ptr<float[]> process(float* carrier, float* modulator) { //owo!

            std::fill(res.get(), res.get() + numSamples, 0.0f);
            for (auto& x : bands) {
                //process band
                x.processModulator(modulator);
            }
            std::copy(carrier, carrier + numSamples, &carrierCopy[0]);
            if (childProcessor != nullptr) {
                //lowpass carrier & modulator
                    float* channel = carrierCopy.get();
                for (auto& x : halfband[0]) {
                    auto tmp = juce::dsp::AudioBlock<float>(&channel, 1, numSamples);
                    x.process(juce::dsp::ProcessContextReplacing<float>(tmp));
                }
                for (auto& x : halfband[1]) {
                    auto tmp = juce::dsp::AudioBlock<float>(&modulator, 1, numSamples);
                    x.process(juce::dsp::ProcessContextReplacing<float>(tmp));
                }

                //downsample carrier & modulator
                for (int i = 0; i < numSamples / 2; i++) {
                    downsampledCarrier[i] = carrierCopy[i * 2];
                    downsampledModulator[i] = modulator[i * 2];
                }

                //process the downsampled
                auto downsampledRes = childProcessor->process(downsampledCarrier.get(), downsampledModulator.get());

                //upsample
                for (int i = 0; i < numSamples; i++) {
                    carrierCopy[i] = (i & 1) ? 0 : downsampledRes[i/2]*2;
                }

                //lowpass upsampled signal
                for (auto& x : halfband[2]) {
                    auto tmp = juce::dsp::AudioBlock<float>(&channel, 1, numSamples);
                    x.process(juce::dsp::ProcessContextReplacing<float>(tmp));
                }

                //add
                for (int i = 0; i < numSamples; i++) {
                    res[i] += carrierCopy[i];
                }
            }
            for (auto& x : bands) {
                //process band
                auto temp = x.process(carrier);

                //add
                for (int i = 0; i < numSamples; i++) {
                    res[i] += temp[i];
                }
            }

            return res;
        }

    private:

        int tick = 0; // 0 or 1, for downsampling
        float delayInSamples = 0.0f;
        double resFreq = 0.0; // returning 
        int numSamples = 0;
        int numBands = 0;
        std::shared_ptr<float[]> res;
        std::shared_ptr<float[]> downsampledCarrier;
        std::shared_ptr<float[]> downsampledModulator;
        std::unique_ptr<float[]> carrierCopy;


        std::shared_ptr<VocoderInternal> childProcessor;

        //create filters... should be dependent on sampleRate??                       ;;;
        std::vector<juce::dsp::IIR::Filter<float>> halfband[3];
        std::vector<band> bands;
    };

    std::atomic<std::shared_ptr<FIRVocoderInternal>> processorLeftFIR, processorRightFIR;
    std::atomic<std::shared_ptr<VocoderInternal>> processorLeft, processorRight;
    std::vector<float> gain;
    std::shared_ptr<float[]> resLeft = nullptr;
    std::shared_ptr<float[]> resRight = nullptr;

    std::vector<VocoderBandData>* bandData = nullptr; 
    
    juce::AudioProcessor* head = nullptr;


    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> carrierLatencyCompensation{ 2048 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> modulatorLatencyCompensation{ 2048 };

    enum {
        Default,
        LinearPhase,
    } mode = Default;
    void updateLatency() {
        if (mode == LinearPhase) {
            head->setLatencySamples(bufferSize2+filterSize);
            carrierLatencyCompensation.setDelay(bufferSize2 + filterSize);
            modulatorLatencyCompensation.setDelay(bufferSize2 + filterSize);
        }
        else if (mode == Default) {
            head->setLatencySamples(bufferSize);
            carrierLatencyCompensation.setDelay(bufferSize);
            modulatorLatencyCompensation.setDelay(bufferSize);
        }
    }

    void initialize() {
        //20.0152312641 is calibrated so that when the buffer size is 1/12 of an octave, it is tuned to A=440 ^^;
        processorLeft.store(std::make_shared<VocoderInternal>(bufferSize, 5, sampleRate, 20.0152312641, sampleRate / 2, bandWidth, parameters, std::make_shared<std::vector<band*>>()), std::memory_order_release);
        processorRight.store(std::make_shared<VocoderInternal>(bufferSize, 5, sampleRate, 20.0152312641, sampleRate / 2, bandWidth, parameters, std::make_shared<std::vector<band*>>()), std::memory_order_release);

        float range = sampleRate / 2.0 / 20.0152312641;
        int numBands = static_cast<int>(log(range) / log(bandWidth) + 0.5f);
        if (true && numBands <= 40) {
            processorLeftFIR.store(std::make_shared<FIRVocoderInternal>(sampleRate, numBands, parameters), std::memory_order_release);
            processorRightFIR.store(std::make_shared<FIRVocoderInternal>(sampleRate, numBands, parameters), std::memory_order_release);
        }

        gain.resize(processorLeft.load(std::memory_order_acquire)->getNumBands());
        updateLatency();
    };
};
