/*
  ==============================================================================

    InputWrapper.h
    Created: 17 Dec 2021 9:29:28pm
    Author:  erich

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class InputWrapper {
public:
    inline bool isKeyDown(int key) {
        return juce::KeyPress::isKeyCurrentlyDown(key);
    }

    inline bool mouseDown() {
        return _mouseDown;
    }

    inline bool rightMouseDown() {
        return _rightMouseDown;
    }

    inline bool mouseWasDown() {
        return _mouseWasDown;
    }

    inline bool rightMouseWasDown() {
        return _rightMouseWasDown;
    }

    inline int mouseX() {
        return _mouseX;
    }

    inline int mouseY() {
        return _mouseY;
    }

    inline int prevMouseX() {
        return _prevMouseX;
    }

    inline int prevMouseY() {
        return _prevMouseY;
    }

    inline bool ctrlDown() {
        return modifiers.isCommandDown();
    }

    inline bool shiftDown() {
        return modifiers.isShiftDown();
    }

    inline bool altDown() {
        return modifiers.isAltDown();
    }

    inline void updateInputs(int __mouseX, int __mouseY) {
        _prevMouseX = _mouseX;
        _prevMouseY = _mouseY;
        _mouseX = __mouseX;
        _mouseY = __mouseY;
        _mouseWasDown = _mouseDown;
        _rightMouseWasDown = _rightMouseDown;
        _mouseDown = modifiers.isLeftButtonDown();
        _rightMouseDown = modifiers.isRightButtonDown();
        modifiers = juce::ModifierKeys::getCurrentModifiers();
    }
    bool isFocused() {
        return _isFocused;
    }
    void setFocused() {
        _isFocused = true;
    }
    void setNotFocused() {
        _isFocused = false;
    }
    bool canFocus() {
        return (mouseDown() || rightMouseDown()) && (!mouseWasDown()) && (!rightMouseWasDown()) && (!isFocused());
    }
private:
    int _mouseX = 0, _mouseY = 0, _prevMouseX = 0, _prevMouseY = 0;
    int _mouseDown, _rightMouseDown, _mouseWasDown = 0, _rightMouseWasDown = 0;
    bool _isFocused = false;
    juce::ModifierKeys modifiers;
};