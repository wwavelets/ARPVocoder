/*
  ==============================================================================

    Slider.h
    Created: 2 Jan 2022 11:12:42am
    Author:  erich

  ==============================================================================
*/

#pragma once
#include "Shaders.h"
#include "RectRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include <algorithm>
class Slider {
public:
    Slider();
    void init(OpenGLWrapper&);
    void setSlider(float, float, float); // x1, x2, y
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
    float x1, x2, y;
    float val; // 0 <= val <= 1
    int clickTick;
    bool sliderFocused, mouseHovered;
    juce::Colour clickColor;
    RectangleRenderer leftBar, rightBar;
    DotRenderer leftDot, rightDot, midDot, clickDot;
    void processInputs(InputWrapper&);
    void renderSlider(OpenGLWrapper&);
};