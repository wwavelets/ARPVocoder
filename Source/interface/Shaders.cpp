/*
  ==============================================================================

    Shaders.cpp
    Created: 19 Oct 2021 6:17:55pm
    Author:  erich

  ==============================================================================
*/

#include "Shaders.h"


namespace shaderTools {
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> makeUniform(OpenGLWrapper& opengl, std::unique_ptr<juce::OpenGLShaderProgram>& shaderProgram, const char* uniformName) {
        if (opengl.context.extensions.glGetUniformLocation(shaderProgram->getProgramID(), uniformName) < 0)
            jassertfalse;
        return std::make_unique<juce::OpenGLShaderProgram::Uniform>(*shaderProgram, uniformName);
    }
}

namespace shaders {
    const char* vertexShader = R"(
            #version 330 core
            
            in vec4 position;      
            out vec2 fragPos;
            
            void main()
            {
                fragPos.xy = position.xy;
                gl_Position.xyzw = position.xyzw;
            }
        )";    
    const char* textureVertexShader = R"(
            #version 330 core
            
            in vec4 position;       
            in vec2 pos2;
            out vec2 textCoord;
            
            void main()
            {
                gl_Position = position;
                textCoord = pos2;
            }
        )";
    const char* fragShader = R"(
        #version 330 core
            
        in vec2 fragPos;
        uniform vec4 color;
        layout(location=0) out vec4 col;
            
        void main()
        {
            col = color;
        }
    )";
    const char* circleFragShader = R"(
        #version 330 core
        
        in vec2 fragPos;
        uniform vec2 position;
        uniform vec4 color;
        uniform float dist1;
        uniform float dist2;
        uniform float leftAngle;
        uniform float rightAngle;
        layout(location=0) out vec4 col;
        
        void main(){
            vec2 adjustedPos = vec2(((fragPos.x + 1.0f)/2.0f)*1000.0f,((fragPos.y + 1.0f)/2.0f)*600.0f);
            vec2 diff = adjustedPos - position;
            float dist = distance(adjustedPos, position);
            float angle = atan(diff.y,diff.x);
            angle += step(angle, leftAngle) * 6.283185f;
            col = color;
            col.a *= smoothstep(dist1-0.5f,dist1 + 0.5f,dist) * smoothstep(dist - 0.5f,dist+0.5f,dist2) * smoothstep(leftAngle-0.01f, leftAngle, angle) * smoothstep(angle-0.01f,angle, rightAngle);
        }
    )";
    const char* dotFragShader = R"(
        #version 330 core
        
        in vec2 fragPos;
        uniform vec2 position;
        uniform vec4 color;
        uniform float dist;
        layout(location=0) out vec4 col;
        
        void main(){
            vec2 adjustedPos = vec2(((fragPos.x + 1.0f)/2.0f)*1000.0f,((fragPos.y + 1.0f)/2.0f)*600.0f);
            float dist2 = distance(adjustedPos, position);
            col = color;
            col.a *= smoothstep(dist2-0.5f, dist2 + 0.5f, dist);
        }
    )";
    const char* textureFragShader = R"(
        #version 330 core
        
        in vec2 textCoord;
        uniform sampler2D tex;
        layout(location=0) out vec4 col;
        
        void main(){
            col = texture(tex, textCoord);
        }
    )";
    const char* sdfFragShader = R"(
        #version 330 core
        
        in vec2 textCoord;
        uniform sampler2D tex;
        uniform vec4 color;
        uniform float size;
        layout(location=0) out vec4 col;
        
        void main(){
            col = color;
            float r = texture(tex, textCoord).r;
            col.a *= smoothstep(-0.1,0.05,(r-0.5f)*size) * step(0.01,r);
        }
    )";
};

OpenGLWrapper::OpenGLWrapper() {

}
OpenGLWrapper::~OpenGLWrapper() {

}
void OpenGLWrapper::destroy() {
    flat.colorUniform.reset();
    flat.shaderProgram->release();


    dot.colorUniform.reset();
    dot.sizeUniform.reset();
    dot.circlePosUniform.reset();
    dot.shaderProgram->release();


    circle.colorUniform.reset();
    circle.circlePosUniform.reset();
    circle.outerSizeUniform.reset();
    circle.innerSizeUniform.reset();
    circle.leftAngleUniform.reset();
    circle.rightAngleUniform.reset();
    circle.shaderProgram->release();

    tex.shaderProgram->release();

    sdf.shaderProgram->release();
    sdf.colorUniform.reset();
    sdf.sizeUniform.reset();

    for (auto& x : textures)
        x.second->release();
    textures.clear();
}
void OpenGLWrapper::init() {
    
    flat.shaderProgram = getShader(vertexShader, fragShader);
    flat.colorUniform = shaderTools::makeUniform(*this, flat.shaderProgram, "color");


    dot.shaderProgram = getShader(vertexShader, dotFragShader);
    dot.colorUniform = shaderTools::makeUniform(*this, dot.shaderProgram, "color");
    dot.sizeUniform = shaderTools::makeUniform(*this, dot.shaderProgram, "dist");
    dot.circlePosUniform = shaderTools::makeUniform(*this, dot.shaderProgram, "position");


    circle.shaderProgram = getShader(vertexShader, circleFragShader);
    circle.colorUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "color");
    circle.circlePosUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "position");
    circle.outerSizeUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "dist2");
    circle.innerSizeUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "dist1");
    circle.leftAngleUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "leftAngle");
    circle.rightAngleUniform = shaderTools::makeUniform(*this, circle.shaderProgram, "rightAngle");
    
    tex.shaderProgram = getShader(textureVertexShader, textureFragShader);

    sdf.shaderProgram = getShader(textureVertexShader, sdfFragShader);
    sdf.colorUniform = shaderTools::makeUniform(*this, sdf.shaderProgram, "color");
    sdf.sizeUniform = shaderTools::makeUniform(*this, sdf.shaderProgram, "size");
}
std::unique_ptr<juce::OpenGLShaderProgram> OpenGLWrapper::getShader(int vertexShader, int fragmentShader) {
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    shaderProgram.reset(new juce::OpenGLShaderProgram(context));
    if (!(shaderProgram->addVertexShader(getVertexShader(vertexShader)) && 
          shaderProgram->addFragmentShader(getFragmentShader(fragmentShader)) && 
          shaderProgram->link()))
        jassertfalse;
    return shaderProgram;
}

const char* OpenGLWrapper::getVertexShader(int id) {
    if (id == vertexShader) return shaders::vertexShader;
    if (id == textureVertexShader) return shaders::textureVertexShader;
    return shaders::vertexShader;
}
const char* OpenGLWrapper::getFragmentShader(int id) {
    if (id == fragShader) return shaders::fragShader;
    if (id == circleFragShader) return shaders::circleFragShader;
    if (id == dotFragShader) return shaders::dotFragShader;
    if (id == textureFragShader) return shaders::textureFragShader;
    if (id == sdfFragShader) return shaders::sdfFragShader;
    return shaders::fragShader;
}
juce::OpenGLTexture* OpenGLWrapper::getTexture(const char* file) {
    if (textures.count(file)) return textures[file].get();
    textures[file] = std::make_unique<juce::OpenGLTexture>();
    auto res = textures[file].get();
    res->bind();
    int sz;
    const char* data = BinaryData::getNamedResource(file, sz);
    res->loadImage(juce::ImageFileFormat::loadFrom(data, sz));
    return res;
}