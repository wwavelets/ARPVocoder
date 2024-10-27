/*
  ==============================================================================

    Shaders.h
    Created: 19 Oct 2021 6:17:55pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define M_PI 3.14159265358979323846264338327950
//memorized this btw :3
#define M_PI_2 1.57079632679

enum vertexShaders {
    vertexShader,
    textureVertexShader
};
enum fragmentShaders {
    fragShader,
    circleFragShader,
    dotFragShader,
    textureFragShader,
    sdfFragShader
};

class OpenGLWrapper {
public:
    OpenGLWrapper();
    ~OpenGLWrapper();
    void destroy();
    void init();
    juce::OpenGLTexture* getTexture(const char*);
    juce::OpenGLContext context;
    std::unique_ptr<juce::OpenGLShaderProgram> getShader(int, int);
    struct Dot {
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> colorUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> sizeUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> circlePosUniform;
    } dot;
    struct Circle {
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> colorUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> circlePosUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> outerSizeUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> innerSizeUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> leftAngleUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> rightAngleUniform;

    } circle;
    struct Flat {
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> colorUniform;
    } flat;
    struct Tex {
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    }tex;
    struct SDF {
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> colorUniform;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> sizeUniform;
    }sdf;
private:
    std::map<std::string, std::unique_ptr<juce::OpenGLTexture>>  textures;
    const char* getVertexShader(int);
    const char* getFragmentShader(int);
};