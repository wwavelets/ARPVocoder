/*
  ==============================================================================

    DuoRadialSlider.h
    Created: 11 Jan 2022 6:06:03pm
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
class DuoRadialSlider {
public:
    DuoRadialSlider();
    void init(OpenGLWrapper&);
    void setSlider(float, float, float); // x, y, size
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
    float val; // 0 <= val <= 1
    int clickTick;
    bool sliderFocused, mouseHovered;
    juce::Colour clickColor;
    CircleRenderer coloredBar, fullBar;
    DotRenderer leftDot, rightDot, centerDot, midDot, clickDot;
    void renderSlider(OpenGLWrapper&);
    void updateInputs(InputWrapper&);
};