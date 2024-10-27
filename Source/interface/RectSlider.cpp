/*
  ==============================================================================

    RectSlider.cpp
    Created: 23 Sep 2024 8:11:22pm
    Author:  erich

  ==============================================================================
*/

#include "RectSlider.h"

RectSlider::RectSlider(){
    val = 0;
    x = y = x2 = y2 = -1;
}
void RectSlider::init(OpenGLWrapper& opengl) {
    upperRect.init(opengl);
    lowerRect.init(opengl);
}
void RectSlider::setSlider(float _x, float _y, float _x2, float _y2, float _defaultVal)  {
    defaultVal = _defaultVal;
    x = _x;y = _y;x2 = _x2;y2 = _y2;
    boundingBox = juce::Rectangle<float>(x, y, x2 - x, y2 - y);
}
void RectSlider::setColor(juce::Colour filledColor, juce::Colour defaultColor) {
    lowerRect.setColor(filledColor);
    upperRect.setColor(defaultColor);
}
void RectSlider::updateInputs(InputWrapper&inputs) {
    int l = inputs.mouseX(), r = inputs.prevMouseX();
    if (l > r) std::swap(l, r);
    // lerp bruh
    float mid = (x + x2) / 2.0f;
    mid = std::clamp(mid, static_cast<float>(l), static_cast<float>(r));

    float lerp = 0.0f;
    if (r - l < 1e-3) lerp = inputs.mouseY();
    else lerp = inputs.prevMouseY()* abs(inputs.mouseX() - mid) / (r - l) + inputs.mouseY() * abs(inputs.prevMouseX() - mid) / (r - l);
    if (inputs.mouseDown() && l <= x2 && x <= r) {
        val = std::clamp( (lerp - y) / (y2 - y),0.0f, 1.0f);
        if (callback != nullptr) {
            callback(val, id);
        }
    }
    if (inputs.rightMouseDown() && l <= x2 && x <= r) {
        val = defaultVal;
        if (callback != nullptr) {
            callback(val, id);
        }
    }
}
void RectSlider::renderSlider(OpenGLWrapper& opengl) {
    upperRect.setRect(opengl, x, y, x2, y+ (y2 - y) * val);
    lowerRect.setRect(opengl, x, y + (y2 - y) * val-1, x2, y + (y2 - y) * val+1);
    upperRect.update(opengl);
    lowerRect.update(opengl);
}
void RectSlider::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    updateInputs(inputs);
    renderSlider(opengl);
}
void RectSlider::destroy(OpenGLWrapper& opengl) {
    upperRect.destroy(opengl);
    lowerRect.destroy(opengl);
}