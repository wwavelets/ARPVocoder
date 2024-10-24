/*
  ==============================================================================

    FullInterface.cpp
    Created: 14 Oct 2021 5:10:01pm
    Author:  erich

  ==============================================================================
*/

#include "FullInterface.h"
//==============================================================================
FullInterface::FullInterface()
{
}
void FullInterface::init(Plugin* _plugin) {
    plugin = _plugin;
    juce::OpenGLPixelFormat pixelFormat;
    pixelFormat.multisamplingLevel = 5;
    opengl.context.setPixelFormat(pixelFormat);
    opengl.context.setRenderer(this);
    opengl.context.setContinuousRepainting(true);
    opengl.context.attachTo(*this);
}
void FullInterface::destroy() {
    opengl.context.setContinuousRepainting(false);
    opengl.context.detach();
}
FullInterface::~FullInterface()
{
    destroy();
}
void FullInterface::paint(juce::Graphics& g) {
    g;
}

void FullInterface::newOpenGLContextCreated()
{
    //initialize stuff here
    opengl.init();
    TextRenderer::initTextures(opengl);
    plugin->init(opengl);
}


void FullInterface::resized() {

}

void FullInterface::renderOpenGL()
{
    juce::gl::glEnable(juce::gl::GL_BLEND);
    juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
    juce::OpenGLHelpers::clear(juce::Colours::pink);
    inputs.updateInputs(getMouseXYRelative().x * 1000 / getWidth(), 600 - getMouseXYRelative().y * 600 / getHeight());


    plugin->update(opengl, inputs);
}

void FullInterface::openGLContextClosing()
{
    //destroying stuff goes here
    plugin->destroy(opengl);
    opengl.destroy();
}