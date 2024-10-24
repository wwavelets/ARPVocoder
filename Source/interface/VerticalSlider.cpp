/*
  ==============================================================================

    VerticalSlider.cpp
    Created: 11 Jan 2022 6:06:43pm
    Author:  erich

  ==============================================================================
*/

#include "VerticalSlider.h"
VerticalSlider::VerticalSlider(){
    val = 0;
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    y1 = -1; y2 = -1; x = -1;
}
void VerticalSlider::init(OpenGLWrapper& opengl) {
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
void VerticalSlider::setSlider(float _y1, float _y2, float _x) {
    y1 = _y1; y2 = _y2; x = _x;
    leftDot.setPos(x,y1);
    rightDot.setPos(x,y2);
}
void VerticalSlider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    clickColor = filledColor;
    leftBar.setColor(filledColor);
    rightBar.setColor(defaultColor);
    leftDot.setColor(filledColor);
    midDot.setColor(filledColor);
    rightDot.setColor(defaultColor);
}
void VerticalSlider::updateInputs(InputWrapper&inputs) {
    if (sliderFocused == true) {
        val = std::min(std::max((inputs.mouseY() - y1) / (y2 - y1), 0.0f), 1.0f);
    }

    mouseHovered = juce::Rectangle<int>(x - 10.0f, y1 - 10.0f, 20.0f, y2 - y1 + 20.0f).contains(inputs.mouseX(), inputs.mouseY());

    if (inputs.mouseDown() == false)
        sliderFocused = false;
    else if (inputs.mouseWasDown() == false && mouseHovered == true) {
        sliderFocused = true;
        clickTick = 1;
    }

}
void VerticalSlider::renderSlider(OpenGLWrapper& opengl) {
    float mid = y1 + (y2 - y1) * val;

    if (mouseHovered || sliderFocused) {
        leftDot.setSize(2.5f);
        rightDot.setSize(2.5f);
        midDot.setSize(5.5f);
        leftBar.setRect(opengl, x - 2.5f, y1, x + 2.5f, mid);
        rightBar.setRect(opengl, x - 2.5f, mid, x + 2.5f, y2);
    }
    else {
        leftDot.setSize(1.5f);
        rightDot.setSize(1.5f);
        midDot.setSize(3.5f);
        leftBar.setRect(opengl, x - 1.5f, y1, x + 1.5f, mid);
        rightBar.setRect(opengl, x - 1.5f, mid, x + 1.5f, y2);
    }


    midDot.setPos(x, mid);
    if (clickTick < uwu::constants::EXPAND_TICKS) {
        clickDot.setCircle(x, mid, (float)clickTick / ((float)uwu::constants::EXPAND_TICKS) * 15.0f + 6.0f);
        clickDot.setColor(clickColor.withMultipliedAlpha((float)(uwu::constants::EXPAND_TICKS - clickTick) / 70.0f * 0.75f));
    }

    leftDot.update(opengl);
    rightDot.update(opengl);
    leftBar.update(opengl);
    rightBar.update(opengl);
    if (clickTick < uwu::constants::EXPAND_TICKS) clickDot.update(opengl);
    midDot.update(opengl);

    if (clickTick < uwu::constants::EXPAND_TICKS) clickTick++;
}
void VerticalSlider::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    updateInputs(inputs);
    renderSlider(opengl);
}
void VerticalSlider::hide() {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
}
void VerticalSlider::destroy(OpenGLWrapper& opengl) {
    leftBar.destroy(opengl);
    rightBar.destroy(opengl);
    leftDot.destroy(opengl);
    rightDot.destroy(opengl);
    midDot.destroy(opengl);
    clickDot.destroy(opengl);
}