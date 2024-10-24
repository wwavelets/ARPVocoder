/*
  ==============================================================================

    DotRenderer.h
    Created: 2 Jan 2022 11:12:19am
    Author:  erich

  ==============================================================================
*/

#pragma once
#include "Shaders.h"
#include "uwu.h"
#include <JuceHeader.h>

class DotRenderer {
public:
    DotRenderer();
    ~DotRenderer();
    void setPos(float, float);
    void setSize(float);
    void setCircle(float, float, float);
    void setColor(juce::Colour);
    void setBounds(float, float, float, float);
    void init(OpenGLWrapper&);
    void update(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
    DotRenderer(DotRenderer&) = default;
private:
    float x = 0, y = 0, size = 0;
    bool hasBounds = false;
    unsigned int vao = 0, vbo = 0, ibo = 0;
    std::vector<juce::Point<float>> vert = {};
    std::vector<unsigned int> idx = {};
    juce::Colour color = juce::Colour();
    juce::Rectangle<float> bounds = juce::Rectangle<float>();
};