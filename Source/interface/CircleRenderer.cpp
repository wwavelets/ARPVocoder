/*
  ==============================================================================

    CircleRenderer.cpp
    Created: 9 Dec 2021 7:33:55pm
    Author:  erich

  ==============================================================================
*/
#include "CircleRenderer.h"

CircleRenderer::CircleRenderer() {
}
CircleRenderer::~CircleRenderer() {

}
void CircleRenderer::setBounds(float _x, float _y, float _x2, float _y2) {
    hasBounds = true;
    bounds = juce::Rectangle<float>(_x, _y, _x2-_x, _y2-_y);
}
void CircleRenderer::setPos(float _x, float _y) {
    x = _x, y = _y;
}
void CircleRenderer::setSize(float _innerSize, float _outerSize) {
    innerSize = _innerSize;
    outerSize = _outerSize;
}
void CircleRenderer::setSize(float _outerSize) {
    innerSize = 0;
    outerSize = _outerSize;
}
void CircleRenderer::setCircle(float _x, float _y, float _outerSize) {
    setPos(_x, _y);
    setSize(_outerSize);
}
void CircleRenderer::setCircle(float _x, float _y, float _innerSize, float _outerSize) {
    setPos(_x, _y);
    setSize(_innerSize, _outerSize);
}
void CircleRenderer::setAngle(float _leftAngle, float _rightAngle) {
    leftAngle = _leftAngle;
    rightAngle = _rightAngle;
    if (rightAngle < leftAngle) rightAngle += (M_PI*2);
}
void CircleRenderer::setColor(juce::Colour _color) {
    color = _color;
}
void CircleRenderer::init(OpenGLWrapper& opengl) {
    hasBounds = false;

    leftAngle = -M_PI;
    rightAngle = M_PI;

    uwu::glTools::genBuffers(opengl, vao, vbo, ibo);
    uwu::glTools::setBufferSizes(opengl, vbo, ibo, 4, 6);
}
void CircleRenderer::update(OpenGLWrapper& opengl) {
    if (hasBounds) uwu::glTools::makeRect(vert, idx, bounds);
    else uwu::glTools::makeRect(vert, idx, x - outerSize, y - outerSize, x + outerSize, y + outerSize);
    uwu::glTools::convert(vert);
    uwu::glTools::makeVAO(opengl, vao, vbo,ibo, vert, idx);
    opengl.circle.shaderProgram->use();

    uwu::glTools::setColorUniform(opengl.circle.colorUniform, color);
    opengl.circle.leftAngleUniform->set(leftAngle);
    opengl.circle.rightAngleUniform->set(rightAngle);
    opengl.circle.outerSizeUniform->set(outerSize);
    opengl.circle.innerSizeUniform->set(innerSize);
    opengl.circle.circlePosUniform->set(x, y);

    uwu::glTools::draw(opengl, vao, idx.size());
}
void CircleRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}