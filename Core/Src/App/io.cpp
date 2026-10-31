#include "io.hpp"

#include "main.h"
#include "stm32u3xx_hal_gpio.h"
#include <cassert>

bool ButtonState::isPressed() const {
    return _isPressed;
}

bool ButtonState::isHeld() const {
    return _isHeld;
}

bool HAL_isButtonPressed(const Button idx) {
    assert(idx == Button::B1 || idx == Button::B2 || idx == Button::B3);

    switch (idx) {
        case Button::B1:
            return HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_SET;
            break;

        case Button::B2:
            return HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_SET;
            break;

        case Button::B3:
            return HAL_GPIO_ReadPin(BUTTON3_GPIO_Port, BUTTON3_Pin) == GPIO_PIN_SET;
            break;

        default:
            return false;
            break;
            
    }
}