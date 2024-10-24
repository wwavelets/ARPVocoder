/*
  ==============================================================================

    ImageRenderer.h
    Created: 4 Feb 2022 7:39:43pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include "Shaders.h"
#include "uwu.h"
class ImageRenderer {
public:
    void init(OpenGLWrapper&);
    void setImage(OpenGLWrapper& opengl, const char*, float = -1, float = -1);
    void setImage(OpenGLWrapper& opengl, juce::OpenGLTexture*, float = -1, float = -1);
    inline void setPos(float _x, float _y) { x = _x, y = _y; };
    inline void setSize(float sz) { size = sz; };
    void update(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
private:
    float size = 1.0;
    float x, y;
    unsigned int vao, vbo, ibo;
    using vertex = std::pair<juce::Point<float>, juce::Point<float>>;
    std::vector<vertex> vert;
    std::vector<unsigned int> idx;
    juce::OpenGLTexture* texture;

};