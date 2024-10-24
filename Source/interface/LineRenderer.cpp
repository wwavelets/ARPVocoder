/*
  ==============================================================================

    LineRenderer.cpp
    Created: 19 Oct 2021 6:17:14pm
    Author:  erich

  ==============================================================================
*/

#include "LineRenderer.h"


LineRenderer::LineRenderer() {

}
LineRenderer::~LineRenderer() {

}
void LineRenderer::getSkeleton(std::vector<juce::Point<float>>&p, float thickness) {
    vert.resize(p.size() * 4 - 4);
    idx.resize((p.size() - 1) * 6);
    for (int i = 0; i + 1 < p.size(); i++) {
        juce::Point<float>& p0 = p[i], & p1 = p[i + 1];
        juce::Point<float> m = p1 - p0;
        juce::Point<float> s = (juce::Point<float>(m.y, -m.x) / m.getDistanceFromOrigin() * thickness); // clockwise 90 degrees
        vert[i * 4 + 0] = p0 - s;
        vert[i * 4 + 1] = p0 + s;
        vert[i * 4 + 2] = p1 - s;
        vert[i * 4 + 3] = p1 + s;
    }
    for (int i = 0; i + 1 < p.size(); i++) {
        idx[i * 6 + 0] = i * 4 + 0;
        idx[i * 6 + 1] = i * 4 + 1;
        idx[i * 6 + 2] = i * 4 + 2;

        idx[i * 6 + 3] = i * 4 + 1;
        idx[i * 6 + 4] = i * 4 + 2;
        idx[i * 6 + 5] = i * 4 + 3;
    }
}

bool LineRenderer::whichSideOfLine(juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> p) {
    return (p.x - p1.x) * (p2.y - p1.y) - (p.y - p1.y) * (p2.x - p1.x) < 0;
}

void LineRenderer::setLine(OpenGLWrapper&opengl, std::vector<juce::Point<float>>& p, float thickness) {
    if (p.size() < 2) return;
    thickness /= 2;

    getSkeleton(p, thickness);

    idx.resize(idx.size() + (p.size() - 2) * 3);
    int ad = (p.size() - 1) * 6;
    for (int i = 0; i + 2 < p.size(); i++) {
        idx[ad + i * 3 + 0] = i * 4 + 2;
        idx[ad + i * 3 + 1] = i * 4 + 3;
        if (whichSideOfLine(p[i], p[i + 1], p[i + 2])) {
            idx[ad + i * 3 + 2] = (i + 1) * 4 + 1;
        }
        else {
            idx[ad + i * 3 + 2] = (i + 1) * 4 + 0;
        }
    }
    uwu::glTools::convert(vert);
    if (p.size() != sz) {
        sz = p.size();
        uwu::glTools::setBufferSizes(opengl, vbo, ibo, vert.size(), idx.size());
    }
    uwu::glTools::makeVAO(opengl, vao, vbo, ibo, vert, idx);
}

void LineRenderer::update(OpenGLWrapper& opengl)
{
    opengl.flat.shaderProgram->use();
    uwu::glTools::setColorUniform(opengl.flat.colorUniform, color);
    uwu::glTools::draw(opengl, vao, idx.size());
}

void LineRenderer::setColor(juce::Colour _color) {
    color = _color;
}

void LineRenderer::init(OpenGLWrapper&opengl) {
    color = juce::Colours::white;
    uwu::glTools::genBuffers(opengl, vao, vbo, ibo);
    sz = -1;
}

void LineRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}