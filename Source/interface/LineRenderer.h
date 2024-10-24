/*
  ==============================================================================

    LineRenderer.h
    Created: 19 Oct 2021 6:17:14pm
    Author:  erich

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "uwu.h"
#include "Shaders.h"

class LineRenderer {
public:
    LineRenderer();
    ~LineRenderer();
    void setLine(OpenGLWrapper&, std::vector<juce::Point<float>>&, float);
    void update(OpenGLWrapper&);
    void init(OpenGLWrapper&);
    void setColor(juce::Colour);
    void destroy(OpenGLWrapper&);
private:
    void getSkeleton(std::vector<juce::Point<float>>&, float);
    static bool whichSideOfLine(juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> p);
    unsigned int vao, vbo, ibo, sz;
    std::vector<juce::Point<float>> vert;
    std::vector<unsigned int> idx;
    juce::Colour color;
};