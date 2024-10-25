/*
  ==============================================================================

    Plugin.h
    Created: 12 Oct 2024 1:01:53pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Interface/Shaders.h"
#include "Interface/uwu.h"
#include "Interface/LineRenderer.h"
#include "Interface/RectRenderer.h"
#include "Interface/CircleRenderer.h"
#include "Interface/TriangleRenderer.h"
#include "Interface/DotRenderer.h"
#include "Interface/InputWrapper.h"
#include "Interface/Slider.h"
#include "Interface/RectSlider.h"
#include "Interface/RadialSlider.h"
#include "Interface/VerticalSlider.h"
#include "Interface/DuoRadialSlider.h"
#include "Interface/ModulationSlider.h"
#include "Interface/LineEditor.h"
#include "Interface/ImageRenderer.h"
#include "Interface/TextRenderer.h"
#include "Interface/SDFRenderer.h"
#include "Interface/DropDown.h"
#include "VocoderRewrite.cpp"

class Plugin 
{
public:
    Plugin(VocoderParameters, juce::AudioProcessor*);
    ~Plugin();

    void init(OpenGLWrapper& opengl);
    void update(OpenGLWrapper& opengl, InputWrapper&inputs);
    void destroy(OpenGLWrapper&opengl);
    void updateSliders();
    void loadPlugin(std::stringstream&);
    void savePlugin(std::stringstream&);
    std::unique_ptr<VocoderProcessor> processor;
    DropDown modulatorType;
private:
    
    VocoderParameters parameters;
    //OpenGLWrapper opengl;
    //InputWrapper inputs;

    TextRenderer dynamics, modulation;

    std::vector<TextRenderer> markers;
    LineEditor carrierMap, modulatorMap;

    std::vector<RectangleRenderer> bars;
    //std::vector<VerticalSlider> sliders[4]; //{"Default", "Pre-Gain", "Post-Gain", "Delay", "Mod. Gain"}

    std::vector<VocoderBandData> bandData;
    std::vector<RectSlider> bandSliders; // bruhh 
    bool interfaceFocused = false;

    TextRenderer view, bands, repeat, mode, modulator;
    DropDown editChoice;
    DropDown numBands;
    DropDown numRepeat;
    DropDown filterMode;

    std::unique_ptr<RadialSlider> formantSlider;
    std::unique_ptr<RadialSlider> carrierBandSlider;
    std::unique_ptr<RadialSlider> modulatorBandSlider;
    std::unique_ptr<RadialSlider> attackSlider;
    std::unique_ptr<RadialSlider> releaseSlider;
    std::unique_ptr<RadialSlider> mixSlider;

    RectangleRenderer FIRUnusedParametersRectangle; // rectangle for unused parameters during fir 

    TextRenderer formantText;
    TextRenderer attackText;
    TextRenderer carrierText;
    TextRenderer modulatorText;
    TextRenderer releaseText;
    TextRenderer mixText;

    bool initialized = false;
    std::condition_variable cv;
    std::mutex m;
    enum {
        Default,
        PreGain,
        PostGain,
        Delay,
        ModGain
    }viewingMode = Default;
    int numBars = 0;
    int countRepeat = 1000;

};
