/*
  ==============================================================================

    TriangleRenderer.h
    Created: 9 Dec 2021 7:33:41pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "uwu.h"
#include "Shaders.h"

class TriangleRenderer {
public:
    TriangleRenderer();
    ~TriangleRenderer();
    void update(OpenGLWrapper&);
    void init(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
    void setColor(juce::Colour);
    void setTri(OpenGLWrapper& opengl, juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> p3);
    inline void setTri(OpenGLWrapper& opengl, float x1, float y1, float x2, float y2, float x3, float y3) {
        setTri(opengl, juce::Point<float>(x1, y1), juce::Point<float>(x2, y2), juce::Point<float>(x3, y3));
    }
private:
    unsigned int vao, vbo, ibo;
    bool hasBuffers;
    std::vector<juce::Point<float>> vert;
    std::vector<unsigned int> idx;
    juce::Colour color;
};