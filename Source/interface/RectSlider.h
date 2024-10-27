/*
  ==============================================================================

    RectSlider.h
    Created: 23 Sep 2024 8:11:22pm
    Author:  erich

  ==============================================================================
*/



#pragma once
#include "Shaders.h"
#include "RectRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include <algorithm>
class RectSlider {
public:
    RectSlider();
    void init(OpenGLWrapper&);
    void setSlider(float, float, float, float, float); // x, y, x2, y2 , defaultval
    void setColor(juce::Colour, juce::Colour = juce::Colour(0xFF404040)); // color, color behind
    void update(OpenGLWrapper&, InputWrapper&);
    void destroy(OpenGLWrapper&);
    void setCallBack(const std::function<void(float, int)>& _callback, int _id) {
        id = _id;
        callback = _callback;
    }
    inline void setVal(float _val) {
        val = _val;
        if (callback != nullptr) {
            callback(val, id);
        }
    }
    void updateInputs(InputWrapper&);
    void renderSlider(OpenGLWrapper&);
private:
    float x, y, x2, y2;
    juce::Rectangle<float> boundingBox;
    float val = 0.0f;
    float defaultVal = 0.0f;

    std::function<void(float, int)> callback = nullptr; int id = 0;
    RectangleRenderer upperRect, lowerRect;

};
