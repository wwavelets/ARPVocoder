#include "uwu.h"

void uwu::glTools::genBuffers(OpenGLWrapper& opengl, unsigned int& vao, unsigned int& vbo, unsigned int& ibo) {
    opengl.context.extensions.glGenBuffers(1, &vbo);
    opengl.context.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);

    opengl.context.extensions.glGenBuffers(1, &ibo);
    opengl.context.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);

    opengl.context.extensions.glGenVertexArrays(1, &vao);
}

void uwu::glTools::deleteBuffers(OpenGLWrapper& opengl, unsigned int& vao, unsigned int& vbo, unsigned int& ibo) {
    opengl.context.extensions.glDeleteBuffers(1, &vbo);
    opengl.context.extensions.glDeleteBuffers(1, &ibo);
    opengl.context.extensions.glDeleteVertexArrays(1, &vao);
}