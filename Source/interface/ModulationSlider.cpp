/*
  ==============================================================================

    ModulationSlider.cpp
    Created: 11 Jan 2022 6:42:20pm
    Author:  erich

  ==============================================================================
*/

#include "ModulationSlider.h"

ModulationSlider::ModulationSlider(){
    val = 0;
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    x = -1;
    y = -1;
    sz = -1;
}

void ModulationSlider::init(OpenGLWrapper& opengl) {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
    fullBar.init(opengl);
    coloredBar.init(opengl);
    clickDot.init(opengl);
}
void ModulationSlider::setSlider(float _x, float _y, float _sz) {
    x = _x, y = _y; sz = _sz;
    coloredBar.setPos(x, y);
    coloredBar.setSize(sz);
    fullBar.setPos(x, y);
    fullBar.setSize(sz);
    clickDot.setPos(x, y);
}

void ModulationSlider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    clickColor = filledColor;
    coloredBar.setColor(filledColor);
    fullBar.setColor(defaultColor);
}
void ModulationSlider::updateInputs(InputWrapper& inputs) {
    if (sliderFocused == true) {
        val += (inputs.mouseX() - inputs.prevMouseX() + inputs.mouseY() - inputs.prevMouseY()) * 0.01f;
        val = std::min(std::max(val, -1.0f), 1.0f);
    }

    float mouseDist = (inputs.mouseX() - x) * (inputs.mouseX() - x) + (inputs.mouseY() - y) * (inputs.mouseY() - y);
    mouseHovered = mouseDist < (sz+5)*(sz+5);

    if (inputs.mouseDown() == false)
        sliderFocused = false;
    else if (inputs.mouseWasDown() == false && mouseHovered == true) {
        sliderFocused = true;
        clickTick = 1;
    }

    if (clickTick < uwu::constants::EXPAND_TICKS) clickTick++;
}

void ModulationSlider::renderSlider(OpenGLWrapper& opengl) {
    float angle = (float)M_PI_2 + val * -(float)M_PI * 2;
    if (angle < 0)angle += M_PI * 2;
    if (val > 0) {
        if (val == 1) coloredBar.setAngle(M_PI_2, M_PI_2 + M_PI * 2); //special case *shrug*
        else coloredBar.setAngle(angle, M_PI_2);
    }
    else {
        coloredBar.setAngle(M_PI_2, angle);
    }
    if (clickTick < uwu::constants::EXPAND_TICKS) clickDot.update(opengl);
    fullBar.update(opengl);
    coloredBar.update(opengl);
    if (clickTick < uwu::constants::EXPAND_TICKS) {
        clickDot.setCircle(x, y, (float)clickTick / ((float)uwu::constants::EXPAND_TICKS) * 15.0f + sz);
        clickDot.setColor(clickColor.withMultipliedAlpha((float)(uwu::constants::EXPAND_TICKS - clickTick) / ((float)uwu::constants::EXPAND_TICKS) * 0.75f));
    }
}

void ModulationSlider::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    updateInputs(inputs);
    renderSlider(opengl);
}

void ModulationSlider::hide() {
    sliderFocused = false;
    mouseHovered = false;
    clickTick = uwu::constants::EXPAND_TICKS;
}

void ModulationSlider::destroy(OpenGLWrapper& opengl) {
    coloredBar.destroy(opengl);
    fullBar.destroy(opengl);
    clickDot.destroy(opengl);
}