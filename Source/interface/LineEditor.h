/*
  ==============================================================================

    LineEditor.h
    Created: 15 Jan 2022 9:35:38pm
    Author:  erich

  ==============================================================================
*/

#pragma once

#include "Shaders.h"
#include "LineRenderer.h"
#include "DotRenderer.h"
#include "InputWrapper.h"
class LineEditor {
public:
    LineEditor();
    ~LineEditor();
    void init(OpenGLWrapper&);
    void destroy(OpenGLWrapper&);
    void setColor(juce::Colour);
    void setLineRenderer(float, float, float, float);
    inline void setLineRenderer(juce::Rectangle<float> x) {
        setLineRenderer(x.getX(), x.getY(), x.getX() + x.getWidth(), x.getY() + x.getHeight());
    }
    inline void setLineRenderer(juce::Point<float> p1, juce::Point<float> p2) {
        setLineRenderer(p1.getX(), p1.getY(), p2.getX(), p2.getY());
    }
    void setPos(float, float);
    float query(float); // [0,1]
    void update(OpenGLWrapper&, InputWrapper&, float);
    void hide();
    void load(std::stringstream&);
    void save(std::stringstream&);
private:
    inline float solve(float x, float w) {
        if (w >= 0) return pow(x, w+1);
        else return 1-pow(1 - x, -w+1);
    }
    inline juce::Point<float> bezier(float w, float t) { //wait this isnt even a bezier curve LOL
        if (w <= 0) t = 1 - pow((1 - t), 3.0f / (-w + 3.0f));
        else t = pow(t, 3.0f / (w + 3.0f));
        return juce::Point<float>(t, solve(t, w));
    }
    void getHovered(InputWrapper&);
    void render(OpenGLWrapper&, float);
    void processInputs(InputWrapper&, OpenGLWrapper&);
    void renderLine(OpenGLWrapper&);
    void pointCreation(InputWrapper&, OpenGLWrapper&);
    void pointDeletion(InputWrapper&, OpenGLWrapper&);
    void movePoints(InputWrapper&);
    void setCurvePoints();
    void precompute();
    
        DotRenderer dot;
    struct point {
        juce::Point<float> pos;
        point() :pos({ 0,0 }) {};
        point(float x, float y) :pos({ x,y }) {};
        point(juce::Point<float> x) :pos(x) {};
        point(const point& x) {
            pos = x.pos;
        }
    };

    struct curvePoint {
        float amt;
        float amt2;
        juce::Point<float> pos;
        curvePoint() :pos({ 0, 0 }), amt(0.0),amt2(0.0) {};
        curvePoint(float x) :amt(x),amt2(0.0) {};
        curvePoint(const curvePoint& x) {
            amt = x.amt;
            amt2 = x.amt2;
            pos = x.pos;
        }
    };

    float xPos, yPos, width, height;
    int hoveredIdx, hoveredType;
    int lastClicked = uwu::constants::DOUBLE_CLICK; 
    float lastMouseX = 0, lastMouseY = 0;
    bool linked = false;

    juce::Rectangle<float> bounds;
    
    LineRenderer line;
    juce::Colour color;

    std::vector<juce::Point<float>> linePoints;

    std::vector<point> points;
    std::vector<float> precomputation;


    std::vector<curvePoint> curvePoints;
};