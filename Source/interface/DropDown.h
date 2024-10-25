/*
  ==============================================================================

    DropDown.h
    Created: 17 Sep 2024 6:08:54pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include "Shaders.h"
#include "RectRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
#include "TextRenderer.h"
#include <algorithm>

class DropDown {
public:
    DropDown();
    void init(OpenGLWrapper&);
    inline void setCallBack(const std::function<void(int)>& _callback) {
        callback = _callback;
    }
    void setChoices(const std::vector<std::string>& _choices, int defaultVal = 0);
    void setDropDown(OpenGLWrapper&, float, float, float); // x, y => bottom left
    void setColor(juce::Colour, juce::Colour);
    void update(OpenGLWrapper&, InputWrapper&);
    void hide();
    void destroy(OpenGLWrapper&);
    //boundary not protected, proceed with caution
    inline void setVal(int _val) {
        val = _val;
        if(callback != nullptr) callback(val);
    }
    inline int getVal() {
        return val;
    }
private:
    float x, y, sz;
    int val;
    float width, height;

    static constexpr int borderWidth = 2;
    static constexpr float textHeight = 115;
    static constexpr float marginX = 40;
    static constexpr float marginY = 10;
    bool changed = false;
    juce::Colour textColor;

    std::vector<std::string> choices;
    std::vector<TextRenderer> choicesText;
    TextRenderer selectedText;

    bool dropdownFocused;
    int currentHovered;
    RectangleRenderer rectInner, rectOuter, rectSelect, rectDivide;

    juce::Rectangle<float> selectedRect;
    std::vector<juce::Rectangle<float>> choiceRects;

    std::function<void(int)> callback = nullptr;

    void processInputs(InputWrapper&, OpenGLWrapper&);
    void renderDropDown(OpenGLWrapper&);
};