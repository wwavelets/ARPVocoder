/*
  ==============================================================================

    SDFRenderer.h
    Created: 23 Feb 2022 11:38:30am
    Author:  erich

  ==============================================================================
*/

#pragma once

#include "Shaders.h"
#include "uwu.h"
class SDFRenderer {
public:
    void init(OpenGLWrapper&);
    void setSDF(OpenGLWrapper& opengl, const char*, float = -1, float = -1);
    void setSDF(juce::OpenGLTexture*, float = -1, float = -1);
    inline void setColor(juce::Colour _color) {
        color = _color;
    };
    inline void setPos(float _x, float _y) { x = _x, y = _y; };
    inline void setSize(float sz) { size = sz; };
    void upd(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
private:
    float size = 1.0;
    float x, y;
    unsigned int vao, vbo, ibo;

    using vertex = std::pair<juce::Point<float>, juce::Point<float>>;
    juce::Colour color;
    std::vector<vertex> vert;
    std::vector<unsigned int> idx;
    juce::OpenGLTexture* texture;

};