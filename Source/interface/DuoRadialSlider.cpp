/*
  ==============================================================================

    DuoRadialSlider.cpp
    Created: 11 Jan 2022 6:06:03pm
    Author:  erich

  ==============================================================================
*/

#include "DuoRadialSlider.h"

DuoRadialSlider::DuoRadialSlider(){
    val = 0.5;
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    sz = -1;
    x = -1;
    y = -1;
}

void DuoRadialSlider::init(OpenGLWrapper& opengl) {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    coloredBar.init(opengl);
    fullBar.init(opengl);
    leftDot.init(opengl);
    midDot.init(opengl);
    rightDot.init(opengl);
    clickDot.init(opengl);
    centerDot.init(opengl);
}
void DuoRadialSlider::setSlider(float _x, float _y, float _sz) {
    x = _x, y = _y, sz = _sz;
    leftDot.setPos(x + sz * -0.70710f, y + sz * -0.70710f);
    rightDot.setPos(x + sz * 0.70710f, y + sz * -0.70710f);
    coloredBar.setPos(x, y);
    fullBar.setPos(x, y);
    centerDot.setPos(x, y + sz);
}

void DuoRadialSlider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    clickColor = filledColor;
    coloredBar.setColor(filledColor);
    fullBar.setColor(defaultColor);
    leftDot.setColor(defaultColor);
    midDot.setColor(filledColor);
    centerDot.setColor(filledColor);
    rightDot.setColor(defaultColor);
}
void DuoRadialSlider::updateInputs(InputWrapper& inputs) {
    if (sliderFocused == true) {
        val += (inputs.mouseX() - inputs.prevMouseX() + inputs.mouseY() - inputs.prevMouseY()) * 0.005f;
        val = std::min(std::max(val, 0.0f), 1.0f);
    }

    float mouseDist = (inputs.mouseX() - x) * (inputs.mouseX() - x) + (inputs.mouseY() - y) * (inputs.mouseY() - y);
    mouseHovered = mouseDist < (sz + 10)* (sz + 10);

    if (inputs.mouseDown() == false)
        sliderFocused = false;
    else if (inputs.mouseWasDown() == false && mouseHovered == true) {
        sliderFocused = true;
        clickTick = 1;
    }
    if (clickTick < uwu::constants::EXPAND_TICKS) clickTick++;
}
void DuoRadialSlider::renderSlider(OpenGLWrapper& opengl) {
    float mid = 3.92699f - 4.71239f * val; // 5/4 pi -  6/4 pi * val
    if (mid < (float)1.57079632679)
        coloredBar.setAngle(mid, (float)1.57079632679);
    else
        coloredBar.setAngle((float)1.57079632679, mid);
    fullBar.setAngle(-0.7854, 3.92699);
    if (mouseHovered || sliderFocused) {
        leftDot.setSize(2.5f);
        rightDot.setSize(2.5f);
        midDot.setSize(5.5f);
        centerDot.setSize(2.5f);
        coloredBar.setSize(sz - 2.5f, sz + 2.5f);
        fullBar.setSize(sz - 2.5f, sz + 2.5f);
    }
    else {
        leftDot.setSize(1.5f);
        rightDot.setSize(1.5f);
        midDot.setSize(3.5f);
        centerDot.setSize(1.5f);
        coloredBar.setSize(sz - 1.5f, sz + 1.5f);
        fullBar.setSize(sz - 1.5f, sz + 1.5f);
    }

    midDot.setPos(x + sz * cos(mid), y + sz * sin(mid));
    if (clickTick < uwu::constants::EXPAND_TICKS) {
        clickDot.setCircle(x + sz * cos(mid), y + sz * sin(mid), (float)clickTick / ((float)uwu::constants::EXPAND_TICKS) * 15.0f + 6.0f);
        clickDot.setColor(clickColor.withMultipliedAlpha((float)(uwu::constants::EXPAND_TICKS - clickTick) / ((float)uwu::constants::EXPAND_TICKS) * 0.75f));
    }

    leftDot.update(opengl);
    rightDot.update(opengl);
    fullBar.update(opengl);
    coloredBar.update(opengl);
    centerDot.update(opengl);
    if (clickTick < uwu::constants::EXPAND_TICKS) clickDot.update(opengl);
    midDot.update(opengl);
}
void DuoRadialSlider::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    updateInputs(inputs);
    renderSlider(opengl);
}
void DuoRadialSlider::hide() {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
}
void DuoRadialSlider::destroy(OpenGLWrapper& opengl) {
    coloredBar.destroy(opengl);
    fullBar.destroy(opengl);
    leftDot.destroy(opengl);
    rightDot.destroy(opengl);
    midDot.destroy(opengl);
    clickDot.destroy(opengl);
    centerDot.destroy(opengl);
}