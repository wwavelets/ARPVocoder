/*
  ==============================================================================

    TextRenderer.cpp
    Created: 21 Feb 2022 11:07:33am
    Author:  erich

  ==============================================================================
*/

#include "TextRenderer.h"

juce::OpenGLTexture *TextRenderer::textures[];

TextRenderer::TextRenderer() {
    x = 0; y = 0; sz = 1.0;
}
TextRenderer::~TextRenderer() {
    
}

void TextRenderer::initTextures(OpenGLWrapper& opengl) {
    for (int i = 0; i < 95; i++) {
        std::string tmp = "_" + std::to_string(i) + "_png";
        textures[i] = opengl.getTexture(tmp.c_str());
    }
}

void TextRenderer::build() {
    float curx = x;
    for (int i = 0; i < sdfs.size(); i++) {
        const char ch = str[i] - ' ';
        sdfs[i].setPos(curx, y-sz * textures[ch]->getHeight());
        sdfs[i].setSize(sz);
        curx += sz * textures[ch]->getWidth();
    }
}
void TextRenderer::setText(OpenGLWrapper& opengl, const std::string& text, float _x, float _y, float _sz) {
    str = text;
    const int len = text.length();
    while (sdfs.size() < len) {
        sdfs.push_back(SDFRenderer());
        sdfs.back().init(opengl);
    }
    while (sdfs.size() > len) {
        sdfs.back().destroy(opengl);
        sdfs.pop_back();
    }
    for (int i = 0; i < len; i++) {
        if (text[i] >= ' ' && text[i] <= '~')
            sdfs[i].setSDF(textures[text[i] - ' ']);
    }
    sz = _sz; x = _x; y = _y;
    build();
}

float TextRenderer::getLength() {
    float res = 0;
    for (auto& x : str) {
        res += sz * textures[x - ' ']->getWidth();
    }
    return res;
}
void TextRenderer::upd(OpenGLWrapper& opengl) {
    for (auto& x : sdfs)
        x.upd(opengl);
}
void TextRenderer::destroy(OpenGLWrapper& opengl) {
    for (auto& x : sdfs)
        x.destroy(opengl);
    sdfs.clear();
}