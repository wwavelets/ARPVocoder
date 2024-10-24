#pragma once

#include <JuceHeader.h>
#include "Shaders.h"

//==============================================================================
/*
*/

namespace uwu {
    namespace constants {
        static const int EXPAND_TICKS = 70;
        static const int DOUBLE_CLICK = 50;
        static const float HOVER_RADIUS = 250;
        static const int NUM_POINTS_CURVE = 24;
    }
    inline float distSq(const float& x, const float& y) {
        return x * x + y * y;
    }
    inline float dist(const float& x, const float& y) {
        return sqrt(x * x + y * y);
    }
    namespace glTools {
        inline void convert(std::vector<juce::Point<float>>& vert) {
            for (auto& v : vert) {
                v.x = v.x / 1000.0f * 2.0f - 1.0f;
                v.y = v.y / 600.0f * 2.0f - 1.0f;
            }
        }

        inline void setColorUniform(std::unique_ptr<juce::OpenGLShaderProgram::Uniform>& colorUniform, juce::Colour& color) {
            colorUniform->set((float)color.getRed() / 255.0f,
                (float)color.getGreen() / 255.0f,
                (float)color.getBlue() / 255.0f,
                (float)color.getAlpha() / 255.0f);
        }

        inline void makeRect(std::vector<juce::Point<float>>& vert, std::vector<unsigned int>& idx, float x1, float y1, float x2, float y2) {
            vert = { {x1,y1},{x1,y2},{x2,y2},{x2,y1} };
            idx = { 0,1,2,0,2,3 };
        }
        inline void makeRect(std::vector<juce::Point<float>>& vert, std::vector<unsigned int>& idx, juce::Point<float>& p1, juce::Point<float>& p2) {
            makeRect(vert, idx, p1.x, p1.y, p2.x, p2.y);
        }
        inline void makeRect(std::vector<juce::Point<float>>& vert, std::vector<unsigned int>& idx, juce::Rectangle <float>& rect) {
            makeRect(vert, idx, rect.getX(), rect.getY(), rect.getX() + rect.getWidth(), rect.getY() + rect.getHeight());
        }
        inline void draw(OpenGLWrapper& opengl, unsigned int& vao, unsigned int sz) {
            opengl.context.extensions.glBindVertexArray(vao);
            juce::gl::glDrawElements(juce::gl::GL_TRIANGLES, sz, juce::gl::GL_UNSIGNED_INT, nullptr);
        }


        template<typename T = juce::Point<float>>
        static void setBufferSizes(OpenGLWrapper& opengl, unsigned int& vbo, unsigned int& ibo, int vertices, int indices) {
            opengl.context.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
            opengl.context.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(T) * vertices, nullptr, juce::gl::GL_STATIC_DRAW);

            opengl.context.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);
            opengl.context.extensions.glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices, nullptr, juce::gl::GL_STATIC_DRAW);
        };
        template<typename T = juce::Point<float>>
        static void makeVAO(OpenGLWrapper& opengl, unsigned int& vao, unsigned int& vbo, unsigned int& ibo, std::vector<T>& vertices, std::vector<unsigned int>& indices, std::vector<int> attributeSizes = { 2 }) {
            // generates a vertex attribute object

            opengl.context.extensions.glBindVertexArray(vao);

            opengl.context.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
            opengl.context.extensions.glBufferSubData(juce::gl::GL_ARRAY_BUFFER, 0, sizeof(T) * vertices.size(), vertices.data());

            opengl.context.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);
            opengl.context.extensions.glBufferSubData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * indices.size(), indices.data());

            int i = 0, t = 0;
            for (auto& x : attributeSizes) {
                opengl.context.extensions.glVertexAttribPointer(i, x, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof(T), (GLvoid*)(sizeof(float) * t));
                opengl.context.extensions.glEnableVertexAttribArray(i);
                i++; t += x;
            }
        };

        void genBuffers(OpenGLWrapper& opengl, unsigned int& vao, unsigned int& vbo, unsigned int& ibo);
        void deleteBuffers(OpenGLWrapper& opengl, unsigned int& vao, unsigned int& vbo, unsigned int& ibo);
    }
}