#pragma once

enum class Button {
    B1 = 0,
    B2,
    B3
};

class ButtonState {
private:
    bool _isPressed;
    bool _isHeld;

    friend void StartInputManagerTask(void *argument);

public:
    ButtonState() : _isPressed(false), _isHeld(false) {};

    bool isPressed() const;
    bool isHeld() const;
};