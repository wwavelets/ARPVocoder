/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                        )
#endif
{
    formatManager.registerBasicFormats();

    addParameter(attack = new juce::AudioParameterFloat("attack", "Attack", juce::NormalisableRange<float>(0.005f, 1.0f, 0, 0.5f), 0.005f));
    addParameter(release = new juce::AudioParameterFloat("release", "Release", juce::NormalisableRange<float>(0.005f, 1.0f, 0, 0.5f), 0.01f));
    addParameter(carrierBandWidth = new juce::AudioParameterFloat("carrier bandwidth", "Carrier Bandwidth", juce::NormalisableRange<float>(0.1f, 4.0f, 0, 0.5f), 1.0f));
    addParameter(modulatorBandWidth = new juce::AudioParameterFloat("modulator bandwidth", "Modulator Bandwidth", juce::NormalisableRange<float>(0.1f, 4.0f, 0, 0.5f), 1.0f));
    addParameter(formant = new juce::AudioParameterFloat("formant shift", "Formant Shift", juce::NormalisableRange<float>(-24.0f, 24.0f), 0.0f));
    addParameter(mix = new juce::AudioParameterFloat("mix", "Mix", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    VocoderParameters parameters;
    parameters.attack = attack;
    parameters.release = release;
    parameters.carrierBandWidth = carrierBandWidth;
    parameters.modulatorBandWidth = modulatorBandWidth;
    parameters.formant = formant;
    parameters.mix = mix;
    plugin = std::make_unique<Plugin>(parameters, this);

    {
        std::lock_guard<std::mutex> lock(m);
        initialized = true;
    }
    cv.notify_one();
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setLatencySamples(0);
    plugin->processor->setSampleRate(sampleRate);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    int bufferSize = buffer.getNumSamples();
    auto main = getBusBuffer (buffer, true, 0);
    auto sidechain = getBusBuffer(buffer, true, 1);

    auto left = main.getWritePointer(0);
    auto right = main.getWritePointer(1);


    if (curState == playing) {
        if (loadedFile.getNumSamples()) {
            for (int i = 0; i < bufferSize; i++) {
                if (currentSample >= loadedFile.getNumSamples()) {
                    currentSample = 0;
                }

                left[i] = loadedFile.getSample(0, currentSample);
                if (loadedFile.getNumChannels() > 1) {
                    right[i] = loadedFile.getSample(1, currentSample);
                }
                else {
                    right[i] = loadedFile.getSample(0, currentSample);
                }
                currentSample++;
            }
        }
    }

    if (curState == start) {
        curState = playing;
    }

    if (plugin->modulatorType.getVal() == 0) {
        plugin->processor->process(main, main);
    }
    else {
        if (sidechain.getNumChannels() == 0) {
            std::fill(left, left + bufferSize, 0.0f);
            std::fill(right, right + bufferSize, 0.0f);
            plugin->processor->process(main, main); //lol
        }
        else {
            plugin->processor->process(main, sidechain);
        }
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this] {return this->initialized; });

    std::stringstream res;
    plugin->savePlugin(res); 
    juce::MemoryOutputStream(destData, true).writeString(res.str());
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this] {return this->initialized; });

    std::string res = juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readString().toStdString();
    std::stringstream in(res);
    plugin->loadPlugin(in);
}

void NewProjectAudioProcessor::loadFile(const juce::File& file) {
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr) {   
        juce::AudioBuffer<float> newBuffer;
        newBuffer.setSize(reader->numChannels, reader->lengthInSamples);
        reader->read(&newBuffer, 0, reader->lengthInSamples, 0, true, true);
        std::swap(loadedFile, newBuffer);
    }

    delete reader;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ 
    return new NewProjectAudioProcessor();
}

