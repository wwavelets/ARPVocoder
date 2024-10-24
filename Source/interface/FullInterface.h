/*
  ==============================================================================

    FullInterface.h
    Created: 14 Oct 2021 5:10:01pm
    Author:  erich

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "Shaders.h"
#include "uwu.h"
#include "LineRenderer.h"
#include "RectRenderer.h"
#include "CircleRenderer.h"
#include "TriangleRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include "Slider.h"
#include "RectSlider.h"
#include "RadialSlider.h"
#include "VerticalSlider.h"
#include "DuoRadialSlider.h"
#include "ModulationSlider.h"
#include "LineEditor.h"
#include "ImageRenderer.h"
#include "TextRenderer.h"
#include "SDFRenderer.h"
#include "DropDown.h"
#include "../VocoderRewrite.cpp"
#include "../Plugin.h"



class FullInterface : public juce::Component, public juce::OpenGLRenderer
{
public:
    FullInterface();
    ~FullInterface() override;

    void resized() override;
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void destroy();
    void init(Plugin*);
    void paint(juce::Graphics&) override;
    void openGLContextClosing() override;
private:
    OpenGLWrapper opengl;
    InputWrapper inputs;

    Plugin* plugin = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FullInterface)
};
