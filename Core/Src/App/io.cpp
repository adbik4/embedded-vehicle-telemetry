#include "io.hpp"

bool ButtonState::isPressed() const {
    return _isPressed;
}

bool ButtonState::isHeld() const {
    return _isHeld;
}