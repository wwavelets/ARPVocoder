/*
  ==============================================================================

    VerticalSlider.h
    Created: 11 Jan 2022 6:06:43pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include "Shaders.h"
#include "RectRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include <algorithm>
class VerticalSlider {
public:
    VerticalSlider();
    void init(OpenGLWrapper&);
    void setSlider(float, float, float); // y1, y2, x
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
    float y1, y2, x;
    float val; // 0 <= val <= 1
    int clickTick;
    bool sliderFocused, mouseHovered;
    juce::Colour clickColor;
    RectangleRenderer leftBar, rightBar;
    DotRenderer leftDot, rightDot, midDot, clickDot;
    void updateInputs(InputWrapper&);
    void renderSlider(OpenGLWrapper&);
};
