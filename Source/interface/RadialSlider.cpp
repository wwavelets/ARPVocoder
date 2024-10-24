/*
  ==============================================================================

    RadialSlider.cpp
    Created: 2 Jan 2022 1:44:29pm
    Author:  erich

  ==============================================================================
*/

#include "RadialSlider.h"

RadialSlider::RadialSlider(juce::AudioParameterFloat* val) : val(val) {
    val = 0;
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    x = -1; y = -1; sz = -1;
}

void RadialSlider::init(OpenGLWrapper& opengl) {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    leftBar.init(opengl);
    rightBar.init(opengl);
    leftDot.init(opengl);
    midDot.init(opengl);
    rightDot.init(opengl);
    clickDot.init(opengl);
}   
void RadialSlider::setSlider(float _x, float _y, float _sz) {
    x = _x, y = _y, sz = _sz;
    leftDot.setPos(x + sz * -0.70710, y + sz * -0.70710);
    rightDot.setPos(x + sz * 0.70710, y + sz * -0.70710);
    leftBar.setPos(x, y);
    rightBar.setPos(x, y);
}

void RadialSlider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    clickColor = filledColor;
    leftBar.setColor(filledColor);
    rightBar.setColor(defaultColor);
    leftDot.setColor(filledColor);
    midDot.setColor(filledColor);
    rightDot.setColor(defaultColor);
}
void RadialSlider::updateInputs(InputWrapper& inputs) {
    if (sliderFocused == true) {
        (*val) = (*val).convertFrom0to1((*val).convertTo0to1((*val)) + (inputs.mouseX() - inputs.prevMouseX() + inputs.mouseY() - inputs.prevMouseY()) * 0.005f);
        (*val) = (*val).convertFrom0to1(std::min(std::max((*val).convertTo0to1((*val)), 0.0f), 1.0f));
    }

    float mouseDist = (inputs.mouseX() - x) * (inputs.mouseX() - x) + (inputs.mouseY() - y) * (inputs.mouseY() - y);
    mouseHovered = mouseDist < (sz + 10)* (sz + 10);

    if (inputs.mouseDown() == false) {
        if (sliderFocused) inputs.setNotFocused();
        sliderFocused = false;
    }
    else if (inputs.mouseWasDown() == false && mouseHovered == true && inputs.canFocus()) {
        sliderFocused = true;
        inputs.setFocused();
        clickTick = 1;
    }

    if (clickTick < uwu::constants::EXPAND_TICKS) clickTick++;
}

void RadialSlider::renderSlider(OpenGLWrapper& opengl) {
    float mid = 3.92699 - 4.71239 * (*val).convertTo0to1(*val); // 5/4 pi -  6/4 pi * val

    leftBar.setAngle(mid, 3.92699);
    rightBar.setAngle(-0.7854, mid);
    if (mouseHovered || sliderFocused) {
        leftDot.setSize(2.5f);
        rightDot.setSize(2.5f);
        midDot.setSize(5.5f);
        leftBar.setSize(sz - 2.5f, sz + 2.5f);
        rightBar.setSize(sz - 2.5f, sz + 2.5f);
    }
    else {
        leftDot.setSize(1.5f);
        rightDot.setSize(1.5f);
        midDot.setSize(3.5f);
        leftBar.setSize(sz - 1.5f, sz + 1.5f);
        rightBar.setSize(sz - 1.5f, sz + 1.5f);
    }


    midDot.setPos(x + sz * cos(mid), y + sz * sin(mid));
    if (clickTick < uwu::constants::EXPAND_TICKS) {
        clickDot.setCircle(x + sz * cos(mid), y + sz * sin(mid), (float)clickTick / ((float)uwu::constants::EXPAND_TICKS) * 15.0f + 6.0f);
        clickDot.setColor(clickColor.withMultipliedAlpha((float)(uwu::constants::EXPAND_TICKS - clickTick) / ((float)uwu::constants::EXPAND_TICKS) * 0.75f));
    }

    leftDot.update(opengl);
    rightDot.update(opengl);
    leftBar.update(opengl);
    rightBar.update(opengl);
    if (clickTick < uwu::constants::EXPAND_TICKS) clickDot.update(opengl);
    midDot.update(opengl);
}
void RadialSlider::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    updateInputs(inputs);
    renderSlider(opengl);
}
void RadialSlider::hide() {
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
}
void RadialSlider::destroy(OpenGLWrapper& opengl) {
    leftBar.destroy(opengl);
    rightBar.destroy(opengl);
    leftDot.destroy(opengl);
    rightDot.destroy(opengl);
    midDot.destroy(opengl);
    clickDot.destroy(opengl);
}