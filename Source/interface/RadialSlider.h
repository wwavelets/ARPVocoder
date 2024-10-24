/*
  ==============================================================================

    RadialSlider.h
    Created: 2 Jan 2022 1:44:29pm
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
class RadialSlider {
public:
    RadialSlider(juce::AudioParameterFloat*);
    void init(OpenGLWrapper&);
    void setSlider(float, float, float); // x, y, size
    void setColor(juce::Colour, juce::Colour = juce::Colour(0xFF404040));
    void update(OpenGLWrapper&, InputWrapper&);
    void hide();
    void destroy(OpenGLWrapper&);
    inline void setVal(float _val) {
        (*val) = _val;
    }
    inline float getVal() {
        return (*val);
    }
private:
    float x, y, sz;
    juce::AudioParameterFloat* val; // 0 <= val <= 1
    int clickTick;
    bool sliderFocused, mouseHovered;
    juce::Colour clickColor;
    CircleRenderer leftBar, rightBar;
    DotRenderer leftDot, rightDot, midDot, clickDot;
    void renderSlider(OpenGLWrapper&);
    void updateInputs(InputWrapper&);
};