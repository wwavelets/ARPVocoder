/*
  ==============================================================================

    Slider.cpp
    Created: 2 Jan 2022 11:12:42am
    Author:  erich

  ==============================================================================
*/

#include "Slider.h"
Slider::Slider() {
    val = 0;
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    x1 = -1; x2 = -1; y = -1;
}
void Slider::init(OpenGLWrapper& opengl) {
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
void Slider::setSlider(float _x1, float _x2, float _y) {
    x1 = _x1; x2 = _x2; y = _y;
    leftDot.setPos(x1, y);
    rightDot.setPos(x2, y);
}
void Slider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    clickColor = filledColor;
    leftBar.setColor(filledColor);
    rightBar.setColor(defaultColor);
    leftDot.setColor(filledColor);
    midDot.setColor(filledColor);
    rightDot.setColor(defaultColor);
}
void Slider::processInputs(InputWrapper& inputs) {
    if (sliderFocused == true) {
        val = std::min(std::max((inputs.mouseX() - x1) / (x2 - x1), 0.0f), 1.0f);
    }
    if (inputs.mouseDown() == false)
        sliderFocused = false;
    else if (inputs.mouseWasDown() == false && mouseHovered == true) {
        sliderFocused = true;
        clickTick = 1;
    }
    mouseHovered = juce::Rectangle<int>(x1 - 10.0f, y - 10.0f, x2 - x1 + 20.0f, 20.0f).contains(inputs.mouseX(), inputs.mouseY());
    if (clickTick < uwu::constants::EXPAND_TICKS) clickTick++;
}
void Slider::renderSlider(OpenGLWrapper& opengl) {
    float mid = x1 + (x2 - x1) * val;

    if (mouseHovered || sliderFocused) {
        leftDot.setSize(2.5f);
        rightDot.setSize(2.5f);
        midDot.setSize(5.5f);
        leftBar.setRect(opengl, x1, y - 2.5f, mid, y + 2.5f);
        rightBar.setRect(opengl, mid, y - 2.5f, x2, y + 2.5f);
    }
    else {
        leftDot.setSize(1.5f);
        rightDot.setSize(1.5f);
        leftBar.setRect(opengl, x1, y - 1.5f, mid, y + 1.5f);
        rightBar.setRect(opengl, mid, y - 1.5f, x2, y + 1.5f);
        midDot.setSize(3.5f);
    }

    midDot.setPos(mid, y);
    if (clickTick < uwu::constants::EXPAND_TICKS) {
        clickDot.setCircle(mid, y, (float)clickTick / ((float)uwu::constants::EXPAND_TICKS) * 15.0f + 6.0f);
        clickDot.setColor(clickColor.withMultipliedAlpha((float)(uwu::constants::EXPAND_TICKS - clickTick) / 70.0f * 0.75f));
    }

    leftDot.update(opengl);
    rightDot.update(opengl);
    leftBar.update(opengl);
    rightBar.update(opengl);
    if (clickTick < uwu::constants::EXPAND_TICKS) clickDot.update(opengl);
    midDot.update(opengl);
}

void Slider::update(OpenGLWrapper&opengl, InputWrapper&inputs) {
    processInputs(inputs);
    renderSlider(opengl);
}
void Slider::hide() {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
}
void Slider::destroy(OpenGLWrapper& opengl) {
    leftBar.destroy(opengl);
    rightBar.destroy(opengl);
    leftDot.destroy(opengl);
    rightDot.destroy(opengl);
    midDot.destroy(opengl);
    clickDot.destroy(opengl);
}