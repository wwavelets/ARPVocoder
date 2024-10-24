/*
  ==============================================================================

    RectangleRenderer.h
    Created: 19 Oct 2021 6:17:14pm
    Author:  erich

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "uwu.h"
#include "Shaders.h"

using namespace uwu::glTools;

class RectangleRenderer {
public:
    RectangleRenderer();
    ~RectangleRenderer();
    void update(OpenGLWrapper&);
    void init(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
    inline void setColor(juce::Colour _color) {
        color = _color;
    }
    void setRect(OpenGLWrapper&, juce::Rectangle<float>);
    inline void setRect(OpenGLWrapper& opengl, float x1, float y1, float x2, float y2) {
        setRect(opengl, juce::Rectangle<float>(x1, y1, x2 - x1, y2 - y1));
    }
    inline void setRect(OpenGLWrapper& opengl, juce::Point<float> p1, juce::Point<float> p2) {
        setRect(opengl, juce::Rectangle<float>(p1, p2));
    }
private:
    unsigned int vao, vbo, ibo;
    std::vector<juce::Point<float>> vert;
    std::vector<unsigned int> idx;
    juce::Colour color;
};