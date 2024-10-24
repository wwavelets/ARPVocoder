/*
  ==============================================================================

    ModulationSlider.h
    Created: 11 Jan 2022 6:42:20pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include "Shaders.h"
#include "RectRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include "CircleRenderer.h"
#include <algorithm>
class ModulationSlider {
public:
    ModulationSlider();
    void init(OpenGLWrapper&);
    void setSlider(float, float, float = 8); // x, y
    void setColor(juce::Colour, juce::Colour = juce::Colour(0xFF404040));
    void update(OpenGLWrapper&, InputWrapper&);
    void hide();
    void destroy(OpenGLWrapper&);
    inline void setVal(float _val) {
        val = _val;
    }
    inline float getVal() {
        return val;
    }
private:
    float x, y, sz;
    float val; // -1 < val < 1
    int clickTick;
    bool sliderFocused, mouseHovered;
    juce::Colour clickColor;
    CircleRenderer coloredBar;
    DotRenderer fullBar, clickDot;
    void renderSlider(OpenGLWrapper&);
    void updateInputs(InputWrapper&);
};