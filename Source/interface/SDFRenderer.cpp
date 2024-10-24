/*
  ==============================================================================

    SDFRenderer.cpp
    Created: 23 Feb 2022 11:38:30am
    Author:  erich

  ==============================================================================
*/

#include "SDFRenderer.h"
void SDFRenderer::init(OpenGLWrapper& opengl) {
    uwu::glTools::genBuffers(opengl, vao, vbo, ibo);
    uwu::glTools::setBufferSizes<vertex>(opengl, vbo, ibo, 4, 6);
}
void SDFRenderer::setSDF(OpenGLWrapper& opengl, const char* file, float _x, float _y) {
    if (_x != -1) setPos(_x, _y);

    texture = opengl.getTexture(file);
}void SDFRenderer::setSDF(juce::OpenGLTexture* _texture, float _x, float _y) {
    if (_x != -1) setPos(_x, _y);

    texture = _texture;
}
void SDFRenderer::upd(OpenGLWrapper& opengl) {
    const float w = texture->getWidth() * size, h = texture->getHeight() * size;
    vert = { {{x,y},{0.0f,0.0f}},
        {{x + w,y},{1.0f,0.0f}},
        {{x + w,y + h},{1.0f,1.0f}},
        {{x,y + h},{0.0f, 1.0f}}
    };        
    for (auto& v : vert) {
        v.first.x = v.first.x / 1000.0f * 2.0f - 1.0f;
        v.first.y = v.first.y / 600.0f * 2.0f - 1.0f;
    }
    idx = { 0,1,2,0,2,3 };
    uwu::glTools::makeVAO<vertex>(opengl, vao, vbo, ibo, vert, idx,{2,2});
    texture->bind();
    opengl.sdf.shaderProgram->use();
    uwu::glTools::setColorUniform(opengl.sdf.colorUniform, color);
    opengl.sdf.sizeUniform->set(size);
    uwu::glTools::draw(opengl, vao, idx.size());
    texture->unbind();
}
void SDFRenderer::destroy(OpenGLWrapper& opengl) {
    uwu::glTools::deleteBuffers(opengl, vao, vbo, ibo);
}