/*
  ==============================================================================

    TextRenderer.h
    Created: 21 Feb 2022 11:07:33am
    Author:  erich

  ==============================================================================
*/

#pragma once
#include "Shaders.h"
#include "uwu.h"
#include "ImageRenderer.h"
#include "SDFRenderer.h"

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();
    static void initTextures(OpenGLWrapper&); // should be called once
    inline void setPos(float _x, float _y) {
        x = _x; y = _y;
        build();
    };
    inline void setSize(float _sz) {
        sz = _sz;
        build();
    };
    inline void setColor(const juce::Colour& color) {
        for (auto& ch : sdfs)
            ch.setColor(color);
    }
    void upd(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
    float getLength();
    void setText(OpenGLWrapper&, const std::string&, float = -1, float = -1, float = 1.0);
private:
    void build();
    static juce::OpenGLTexture *textures[95];
    const static float xshift[95];
    const static float yshift[95];
    const static float advance[95];
    std::string str;
    float x, y, sz;
    std::vector<SDFRenderer> sdfs;
};