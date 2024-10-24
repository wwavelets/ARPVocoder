/*
  ==============================================================================

    CircleRenderer.h
    Created: 9 Dec 2021 7:33:55pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Shaders.h"
#include "uwu.h"


class CircleRenderer {
public:
    CircleRenderer();
    ~CircleRenderer();
    void setPos(float, float);
    void setSize(float);
    void setSize(float, float);
    void setCircle(float, float, float);
    void setCircle(float, float, float, float);
    void setAngle(float, float);
    void setColor(juce::Colour);
    void setBounds(float, float, float, float);
    void init(OpenGLWrapper&);
    void update(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
private:
    float x, y, innerSize, outerSize, leftAngle, rightAngle;
    bool hasBounds = false;
    unsigned int vao, vbo, ibo;
    std::vector<juce::Point<float>> vert;
    std::vector<unsigned int> idx;
    juce::Colour color;
    juce::Rectangle<float> bounds;
};