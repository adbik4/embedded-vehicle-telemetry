#pragma once

#include <cstdint>

enum class Button {
    B1 = 0,
    B2,
    B3
};

class ButtonState {
private:
    bool _isPressed;
    bool _isHeld;

    friend class SystemState;

public:
    ButtonState() : _isPressed(false), _isHeld(false) {};

    bool isPressed() const;
    bool isHeld() const;
};

bool HAL_isButtonPressed(const Button idx);
Button pinIdToButton(uint32_t pin_id);