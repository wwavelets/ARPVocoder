/*
  ==============================================================================

    LineEditor.cpp
    Created: 15 Jan 2022 9:35:38pm
    Author:  erich

  ==============================================================================
*/

#include "LineEditor.h"

LineEditor::LineEditor() {
    points = { {0.0,0.0},{1.0,1.0} };
    color = juce::Colours::red;
    curvePoints = { 0.0 };
    hoveredIdx = -1;
    lastMouseX = -1000;
    lastMouseY = -1000;
    precomputation = std::vector<float>(500, 0); //lets say 500...
    precompute();
}

LineEditor::~LineEditor() {

}

void LineEditor::precompute() {
    int currentIdx = 0; float tmp = 0;
    for (int i = 0; i < 500; i++) {
        float currentPos = std::pow(tmp, 0.333f);
        while (currentIdx + 1 < points.size() && currentPos > points[currentIdx + 1].pos.x) {
            currentIdx += 1;
        }

        if (currentIdx + 1 < points.size()) {
            if (points[currentIdx + 1].pos.x == points[currentIdx].pos.x) precomputation[i] = points[currentIdx].pos.y;
            else precomputation[i] = points[currentIdx].pos.y + solve((currentPos-points[currentIdx].pos.x) / (points[currentIdx +1].pos.x - points[currentIdx].pos.x), curvePoints[currentIdx].amt) * (points[currentIdx +1].pos.y-points[currentIdx].pos.y);
        }
        else {
            precomputation[i] = points[currentIdx].pos.y;
        }
        precomputation[i] = std::pow(precomputation[i], 3.0f);
        tmp += (1.0f / 499.0f);
    }
}
void LineEditor::getHovered(InputWrapper& inputs) {
    if (inputs.mouseDown()) return;
    float closest = 1000000000; hoveredIdx = -1; hoveredType = -1;
    for (int i = 0; i < points.size(); i++) {
        float px = points[i].pos.x * width + xPos;
        float py = points[i].pos.y * height + yPos;
        float dist = uwu::distSq((inputs.mouseX() - px), (inputs.mouseY() - py) * (inputs.mouseY() - py));
        if (dist < closest) {
            closest = dist;
            hoveredIdx = i;
            hoveredType = 0;
        }
    }

    for (int i = 0; i < curvePoints.size(); i++) {
        float px = curvePoints[i].pos.x * width + xPos;
        float py = curvePoints[i].pos.y * height + yPos;
        float dist = uwu::distSq((inputs.mouseX() - px), (inputs.mouseY() - py));
        if (dist < closest) {
            closest = dist;
            hoveredIdx = i;
            hoveredType = 1;
        }
    }

    if (closest > uwu::constants::HOVER_RADIUS) {
        if (bounds.contains(juce::Point<float>((float)inputs.mouseX(), (float)inputs.mouseY()))){
            hoveredIdx = 0;
            hoveredType = 2;
        }
        else {
            hoveredIdx = -1; hoveredType = -1;
        }
    }
}

void LineEditor::pointCreation(InputWrapper& inputs, OpenGLWrapper&opengl) {
    if (hoveredType != 2) return;
    if (!inputs.mouseDown() || inputs.mouseWasDown())return;
    if (lastClicked < uwu::constants::DOUBLE_CLICK) {
        //add a point
        float mx = (inputs.mouseX() - xPos) / width;
        float my = (inputs.mouseY() - yPos) / height;
        int idx = 1;
        while (idx < points.size()) {
            if (points[idx].pos.x >= mx) {
                break;
            }
            idx++;
        }
        points.insert(points.begin() + idx, juce::Point<float>(mx, my));
        //points[idx].dot.init(opengl);

        curvePoints.insert(curvePoints.begin() + idx, 0.0);
        //curvePoints[idx].dot.init(opengl);

        hoveredType = 0;
        hoveredIdx = idx;
        lastClicked = uwu::constants::DOUBLE_CLICK;
        precompute();
    }
    else {
        lastClicked = 0;
        lastMouseX = (float) inputs.mouseX();
        lastMouseY = (float)inputs.mouseY();
    }
}

void LineEditor::pointDeletion(InputWrapper& inputs, OpenGLWrapper& opengl) {
    if (hoveredType != 0) return;
    if (hoveredIdx == 0 || hoveredIdx == points.size() - 1) return;
    if (!inputs.mouseDown() || inputs.mouseWasDown()) return;
    if (lastClicked < uwu::constants::DOUBLE_CLICK) {
        //points[hoveredIdx].dot.destroy(opengl);
        //curvePoints[hoveredIdx].dot.destroy(opengl);

        points.erase(points.begin() + hoveredIdx);
        curvePoints.erase(curvePoints.begin() + hoveredIdx);

        hoveredType = -1;
        lastClicked = uwu::constants::DOUBLE_CLICK;
        precompute();
    }
    else {
        lastClicked = 0;
        lastMouseX = (float) inputs.mouseX();
        lastMouseY = (float) inputs.mouseY();
    }
}

void LineEditor::movePoints(InputWrapper& inputs) {
    if (!inputs.mouseDown() || !inputs.mouseWasDown() || (inputs.mouseX() == inputs.prevMouseX() && inputs.mouseY() == inputs.prevMouseY())) return;
    if (hoveredType == -1) return;
    if (hoveredType == 0) {
        if ((hoveredIdx == 0 || hoveredIdx == points.size() - 1)) {
            if (linked) {
                points[0].pos.y = (inputs.mouseY() - yPos) / height;
                points[0].pos.y = std::max(0.0f, std::min(points[0].pos.y, 1.0f));
                points.back().pos.y = (inputs.mouseY() - yPos) / height;
                points.back().pos.y = std::max(0.0f, std::min(points.back().pos.y, 1.0f));
            }
            else if (hoveredIdx == 0) {
                points[0].pos.y = (inputs.mouseY() - yPos) / height;
                points[0].pos.y = std::max(0.0f, std::min(points[0].pos.y, 1.0f));
            }
            else {
                points.back().pos.y = (inputs.mouseY() - yPos) / height;
                points.back().pos.y = std::max(0.0f, std::min(points.back().pos.y, 1.0f));
            }
        }
        else {
            points[hoveredIdx].pos.y = (inputs.mouseY()-yPos) / height;
            points[hoveredIdx].pos.y = std::max(0.0f, std::min(points[hoveredIdx].pos.y, 1.0f));

            points[hoveredIdx].pos.x = (inputs.mouseX() - xPos) / width;
            points[hoveredIdx].pos.x = std::max(points[hoveredIdx - 1].pos.x, std::min(points[hoveredIdx].pos.x, points[hoveredIdx + 1].pos.x));
        }
    }
    if (hoveredType == 1) {
        if (points[hoveredIdx].pos.y > points[hoveredIdx + 1].pos.y) {
            curvePoints[hoveredIdx].amt2 += (inputs.mouseY() - inputs.prevMouseY()) / height * 2.5f;
        }
        else {
            curvePoints[hoveredIdx].amt2 -= (inputs.mouseY() - inputs.prevMouseY()) / height * 2.5f;
        }
        curvePoints[hoveredIdx].amt2 = std::max(-2.0f, std::min(curvePoints[hoveredIdx].amt2, 2.0f));

        curvePoints[hoveredIdx].amt = curvePoints[hoveredIdx].amt2 + curvePoints[hoveredIdx].amt2 * curvePoints[hoveredIdx].amt2 * curvePoints[hoveredIdx].amt2 * curvePoints[hoveredIdx].amt2 * curvePoints[hoveredIdx].amt2;
    }
    precompute();
}

void LineEditor::setCurvePoints() {
    jassert(points.size() == curvePoints.size() + 1);
    for (int i = 0; i < curvePoints.size(); i++) {
        float mid = (points[i].pos.x + points[i + 1].pos.x) / 2;
        curvePoints[i].pos.x = mid;
        float tmp = solve(0.5, curvePoints[i].amt);
        curvePoints[i].pos.y = points[i].pos.y + (points[i + 1].pos.y - points[i].pos.y) * tmp;
    }
}

void LineEditor::processInputs(InputWrapper&inputs, OpenGLWrapper&opengl) {
    bounds = juce::Rectangle<float>(xPos, yPos, width, height);
    setCurvePoints();
    getHovered(inputs);
    pointDeletion(inputs, opengl);
    pointCreation(inputs, opengl);
    movePoints(inputs);
    setCurvePoints();

    if (uwu::distSq(inputs.mouseX() - lastMouseX, inputs.mouseY() - lastMouseY) > 25) {
        lastClicked = uwu::constants::DOUBLE_CLICK; // not last clicked
    }

    if (lastClicked < uwu::constants::DOUBLE_CLICK) lastClicked++;
}

void LineEditor::renderLine(OpenGLWrapper& opengl) {
    if (false && points.size() == 2) { // default, no curve editor
        linePoints = { {xPos,yPos + height * points[0].pos.y},{xPos + width,yPos + height * points[1].pos.y} };
        line.setLine(opengl, linePoints, 2.5f);
        line.update(opengl);
    }
    else {
        if (linePoints.size() != uwu::constants::NUM_POINTS_CURVE * curvePoints.size()+1) {
            linePoints.resize(uwu::constants::NUM_POINTS_CURVE * curvePoints.size()+1); // uwus
        }
        for (int i = 0; i < curvePoints.size(); i++) {
            for (int j = 0; j < uwu::constants::NUM_POINTS_CURVE; j++) {
                int idx = i * uwu::constants::NUM_POINTS_CURVE + j;
                linePoints[idx] = bezier(curvePoints[i].amt,(float)j/uwu::constants::NUM_POINTS_CURVE);
                linePoints[idx].x = points[i].pos.x + linePoints[idx].x * (points[i+1].pos.x - points[i].pos.x); // lerp
                linePoints[idx].y = points[i].pos.y + linePoints[idx].y * (points[i+1].pos.y - points[i].pos.y); // lerp

                linePoints[idx].x = linePoints[idx].x * width + xPos;
                linePoints[idx].y = linePoints[idx].y * height + yPos;
            }
        }
        linePoints.back() = points.back().pos;
        linePoints.back().x = linePoints.back().x * width + xPos;
        linePoints.back().y = linePoints.back().y * height + yPos;

        line.setLine(opengl,linePoints, 2.5f);
        line.update(opengl);
    }
}

void LineEditor::render(OpenGLWrapper&opengl, float pos) {
    for (int i = 0; i < points.size(); i++) {
        dot.setPos(points[i].pos.x * width + xPos, points[i].pos.y * height + yPos);
        dot.setColor(color);
        if (hoveredType == 0 && hoveredIdx == i) dot.setSize(7.5f);
        else dot.setSize(5.0f);
        dot.update(opengl);
    }

    for (int i = 0; i < curvePoints.size(); i++) {
        dot.setPos(curvePoints[i].pos.x * width + xPos, curvePoints[i].pos.y * height + yPos);
        dot.setColor(color);
        if (hoveredType == 1 && hoveredIdx == i) dot.setSize(5.0f);
        else dot.setSize(3.5f);
        dot.update(opengl);
    }

    renderLine(opengl);
}


void LineEditor::update(OpenGLWrapper& opengl, InputWrapper& inputs, float pos) {
    processInputs(inputs, opengl);
    render(opengl, pos);

}

void LineEditor::init(OpenGLWrapper& opengl) {
    //for (auto& x : points) x.dot.init(opengl);
    //for (auto& x : curvePoints) x.dot.init(opengl);
    dot.init(opengl);
    line.init(opengl);
}

void LineEditor::destroy(OpenGLWrapper& opengl) {
    linePoints.clear();
    line.destroy(opengl);
    dot.destroy(opengl);
    //for (auto& x : points)
    //    x.dot.destroy(opengl);
    //for (auto& x : curvePoints)
    //    x.dot.destroy(opengl);
}


void LineEditor::setColor(juce::Colour _color) {
    color = _color;
    //for (auto& x : points)
    //    x.dot.setColor(color);
    //for (auto& x : curvePoints)
    //    x.dot.setColor(color);
    dot.setColor(color);
    line.setColor(color);
}

void LineEditor::setLineRenderer(float _x, float _y, float _x2, float _y2) {
    xPos = _x; yPos = _y; width = _x2 - _x; height = _y2 - _y;
}

void LineEditor::setPos(float x, float y) {
    xPos = x; yPos = y;
}

float LineEditor::query(float pos) {
    pos = std::clamp(pos, 0.0f, 1.0f);
    int v = static_cast<int>(pos * 249.0f);
    if (v == 249) return precomputation[v];
    constexpr float c = 1.0f / 249.0f;
    float a = v * c;
    float b = a + c;
    return ((pos - a) * precomputation[v + 1] + (b - pos) * precomputation[v])/c;
}

void LineEditor::hide() {
    hoveredIdx = -1;
    lastMouseX = -1000;
    lastMouseY = -1000;
}


void LineEditor::load(std::stringstream& in) {
    
    int numPoints;
    in >> numPoints;
    points.resize(numPoints);
    for (auto& x : points) {
        in >> x.pos.x >> x.pos.y;
    }
    int numCurvePoints;
    in >> numCurvePoints;
    curvePoints.resize(numCurvePoints);
    for (auto& x : curvePoints) {
        in >> x.amt2;
        x.amt = x.amt2 + x.amt2 * x.amt2 * x.amt2 * x.amt2 * x.amt2;
    }
    setCurvePoints();
    precompute();
}

void LineEditor::save(std::stringstream& out) {
    out << points.size() << "\n";
    for (auto& x : points) {
        out << x.pos.x << " " << x.pos.y << "\n";
    }
    out << curvePoints.size() << "\n";
    for (auto& x : curvePoints) {
        out << x.amt2 << "\n";
    }
}