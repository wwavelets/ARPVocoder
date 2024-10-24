/*
  ==============================================================================

    DotRenderer.cpp
    Created: 2 Jan 2022 11:12:19am
    Author:  erich

  ==============================================================================
*/

#include "DotRenderer.h"

DotRenderer::DotRenderer():ibo(0),size(0),vao(0),vbo(0),x(0),y(0),hasBounds(false),bounds(juce::Rectangle<float>()),

vert({}), idx({}),color(juce::Colour()) {

}
DotRenderer::~DotRenderer() {

}
void DotRenderer::setBounds(float _x, float _y, float _x2, float _y2) {
    hasBounds = true;
    bounds = juce::Rectangle<float>(_x, _y, _x2 - _x, _y2 - _y);
}
void DotRenderer::setPos(float _x, float _y) {
    x = _x, y = _y;
}
void DotRenderer::setSize(float _size) {
    size = _size;
}
void DotRenderer::setCircle(float _x, float _y, float _size) {
    setPos(_x, _y);
    setSize(_size);
}
void DotRenderer::setColor(juce::Colour _color) {
    color = _color;
}
void DotRenderer::init(OpenGLWrapper& opengl) {
    hasBounds = false;
    //shaderProgram = opengl.getShader(vertexShader, dotFragShader);

    uwu::glTools::genBuffers(opengl, vao, vbo, ibo);
    uwu::glTools::setBufferSizes(opengl, vbo, ibo, 4, 6);
}
void DotRenderer::update(OpenGLWrapper& opengl) {

    if (hasBounds) uwu::glTools::makeRect(vert, idx, bounds);
    else uwu::glTools::makeRect(vert, idx, x - size, y - size, x + size, y + size);
    uwu::glTools::convert(vert);
    uwu::glTools::makeVAO(opengl, vao, vbo, ibo, vert, idx);
    opengl.dot.shaderProgram->use();

    uwu::glTools::setColorUniform(opengl.dot.colorUniform, color);
    opengl.dot.sizeUniform->set(size);
    opengl.dot.circlePosUniform->set(x, y);


    uwu::glTools::draw(opengl, vao, idx.size());
}

void DotRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}