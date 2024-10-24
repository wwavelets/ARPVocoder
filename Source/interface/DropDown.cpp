/*
  ==============================================================================

    DropDown.cpp
    Created: 17 Sep 2024 6:08:54pm
    Author:  erich

  ==============================================================================
*/

#include "DropDown.h"

DropDown::DropDown() {
    val = 0;
    dropdownFocused = false;
    currentHovered = -1;
    changed = false;
    textColor = juce::Colour();
    x = -1; y = -1; width = 0; height = 0;
}
void DropDown::init(OpenGLWrapper& opengl) {
    dropdownFocused = false;
    currentHovered = -1;
    rectInner.init(opengl);
    rectOuter.init(opengl);
    rectSelect.init(opengl);
    rectDivide.init(opengl);
}

void DropDown::setChoices(const std::vector<std::string>& _choices, int defaultVal) {
    val = defaultVal;
    choices = _choices;
    if (callback != nullptr) callback(val);
    changed = true;
}
//_x, _y is top left
void DropDown::setDropDown(OpenGLWrapper& opengl, float _x, float _y, float _sz) {
    x = _x; y = _y; sz = _sz;
    choicesText.resize(choices.size());
    width = 0; height = 10.f *sz + textHeight*sz;

    selectedText.setText(opengl, choices[val], x + sz * marginX, y - 10.f * sz - marginY * sz, sz);
    selectedText.setColor(textColor);

    for (int i = 0; i < static_cast<int>(choices.size()); i++) {
        choicesText[i].setText(opengl, choices[i], x + sz * marginX, y-(i+1)*sz* textHeight - 10.f *sz - marginY*sz, sz); // adjust constants
        choicesText[i].setColor(textColor);

        width = std::max(width, choicesText[i].getLength() + sz * (marginX*2));
        height += sz * textHeight;
    }

    choiceRects.resize(choices.size());
    for (int i = 0; i < static_cast<int>(choices.size()); i++) {
        choiceRects[i] = juce::Rectangle<float>(x+borderWidth, y - (i + 2) * sz * textHeight, width-borderWidth*2, textHeight*sz);
    }
    choiceRects.back().setY(choiceRects.back().getY() - 10.f * sz + borderWidth);
    choiceRects.back().setHeight(choiceRects.back().getHeight() + 10.f * sz - borderWidth);

    selectedRect = juce::Rectangle<float>(x + borderWidth, y - textHeight * sz + borderWidth, width - borderWidth * 2, textHeight * sz + 0 * sz - borderWidth * 2);
    rectDivide.setRect(opengl, juce::Rectangle<float>(x + borderWidth, y - textHeight * sz, width - borderWidth * 2, borderWidth));

    changed = false;
}
void DropDown::setColor(juce::Colour _textColor, juce::Colour innerColor) {
    textColor = _textColor;
    rectInner.setColor(innerColor);
    rectOuter.setColor(textColor);
    for (auto& text : choicesText) {
        text.setColor(textColor);
    }
    selectedText.setColor(textColor);

    rectSelect.setColor(innerColor.darker(0.1));
    rectDivide.setColor(innerColor.darker(0.2));
}
void DropDown::processInputs(InputWrapper& inputs, OpenGLWrapper&opengl) {
    if (changed) {
        selectedText.destroy(opengl);
        for (auto& text : choicesText) text.destroy(opengl);
        setDropDown(opengl, x, y, sz);
    }


    //get hovered
    currentHovered = -1;
    if (selectedRect.contains(inputs.mouseX(), inputs.mouseY())) {
        currentHovered = -2;
    }
    for (int i = 0; i < static_cast<int>(choiceRects.size()); i++) {
        if (choiceRects[i].contains(inputs.mouseX(), inputs.mouseY())) {
            currentHovered = i;
        }
    }


    //mouse stuff
    if (inputs.mouseDown() && !inputs.mouseWasDown() && (!inputs.isFocused()||dropdownFocused)) {
        if (currentHovered == -1) {
            if (dropdownFocused) {
                dropdownFocused = false;
                inputs.setNotFocused();
            }
        }
        else if (currentHovered == -2) {
            if (dropdownFocused) {
                dropdownFocused = false;
                inputs.setNotFocused();
            }
            else {
                dropdownFocused = true;
                inputs.setFocused();
            }
        }
        else if (dropdownFocused) {
            if (dropdownFocused) {
                dropdownFocused = false;
                inputs.setNotFocused();
            }
            val = currentHovered;
            selectedText.destroy(opengl);
            selectedText.setText(opengl, choices[val], x + sz * marginX, y - 10.f * sz - marginY * sz, sz);
            selectedText.setColor(textColor);
            if (callback != nullptr) {
                callback(val);
            }
        }
    }
}
void DropDown::renderDropDown(OpenGLWrapper& opengl) {
    if (dropdownFocused) {
        rectOuter.setRect(opengl, x, y - height, x + width, y);
        rectInner.setRect(opengl, x + borderWidth, y - height + borderWidth, x + width - borderWidth, y - borderWidth);
        rectOuter.update(opengl);
        rectInner.update(opengl);

        if (currentHovered >= 0) {
            rectSelect.setRect(opengl, choiceRects[currentHovered]);
            rectSelect.update(opengl);
        }

        if (currentHovered == -2) {
            rectSelect.setRect(opengl, selectedRect);
            rectSelect.update(opengl);
        }

        rectDivide.update(opengl);

        for (auto& text : choicesText) {
            text.upd(opengl);
        }
    }
    else {
        rectOuter.setRect(opengl, x, y - textHeight * sz, x + width, y);
        rectInner.setRect(opengl, x + borderWidth, y - textHeight * sz + borderWidth, x + width - borderWidth, y - borderWidth);
        rectOuter.update(opengl);
        rectInner.update(opengl);

        if (currentHovered == -2) {
            rectSelect.setRect(opengl, selectedRect);
            rectSelect.update(opengl);
        }

    }
    
    selectedText.upd(opengl);
}

void DropDown::update(OpenGLWrapper& opengl, InputWrapper& inputs) {
    processInputs(inputs, opengl);
    renderDropDown(opengl);
}
void DropDown::hide() {
    //dropdownFocused = false;
    currentHovered = -1;
}
void DropDown::destroy(OpenGLWrapper& opengl) {
    rectInner.destroy(opengl);
    rectOuter.destroy(opengl);
    rectSelect.destroy(opengl);
    rectDivide.destroy(opengl);

    selectedText.destroy(opengl);
    for (auto& text : choicesText) text.destroy(opengl);
}