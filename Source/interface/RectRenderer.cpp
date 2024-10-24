/*
  ==============================================================================

    RectangleRenderer.cpp
    Created: 11 Nov 2021 3:46:46pm
    Author:  erich

  ==============================================================================
*/

#include "RectRenderer.h"

RectangleRenderer::RectangleRenderer() {

}

RectangleRenderer::~RectangleRenderer() {

}
void RectangleRenderer::update(OpenGLWrapper&opengl) {
    opengl.flat.shaderProgram->use();
    uwu::glTools::setColorUniform(opengl.flat.colorUniform, color);
    uwu::glTools::draw(opengl, vao, idx.size());
}
void RectangleRenderer::init(OpenGLWrapper& opengl) {
    uwu::glTools::genBuffers(opengl,vao, vbo, ibo);
    uwu::glTools::setBufferSizes(opengl, vbo, ibo, 4, 6);
}
void RectangleRenderer::setRect(OpenGLWrapper& opengl, juce::Rectangle<float> rect) {
    uwu::glTools::makeRect(vert, idx, rect);
    uwu::glTools::convert(vert);
    uwu::glTools::makeVAO<juce::Point<float>>(opengl, vao, vbo, ibo, vert, idx);
}

void RectangleRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}
