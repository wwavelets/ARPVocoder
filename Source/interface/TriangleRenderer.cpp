/*
  ==============================================================================

    TriangleRenderer.cpp
    Created: 9 Dec 2021 7:33:41pm
    Author:  erich

  ==============================================================================
*/

#include "TriangleRenderer.h"


TriangleRenderer::TriangleRenderer() {

}
TriangleRenderer::~TriangleRenderer() {
}
void TriangleRenderer::update(OpenGLWrapper& opengl) {
    opengl.flat.shaderProgram->use();
    uwu::glTools::setColorUniform(opengl.flat.colorUniform, color);
    uwu::glTools::draw(opengl, vao, idx.size());
}
void TriangleRenderer::init(OpenGLWrapper& opengl) {
    uwu::glTools::genBuffers(opengl, vao, vbo, ibo);
    uwu::glTools::setBufferSizes(opengl, vbo, ibo, 3, 3);
}
void TriangleRenderer::setColor(juce::Colour _color) {
    color = _color;
}
void TriangleRenderer::setTri(OpenGLWrapper& opengl, juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> p3) {
    vert = { p1, p2, p3 };
    idx = { 0,1,2 };
    uwu::glTools::convert(vert);
    uwu::glTools::makeVAO(opengl, vao, vbo, ibo, vert, idx);
}
void TriangleRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}