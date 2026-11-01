#include "io.hpp"

#include <cassert>
#include <cstdint>

#if !defined(UNIT_TEST)
    #include "main.h"
    #include "stm32u3xx_hal_gpio.h"
#endif

bool ButtonState::isPressed() const {
    return _isPressed;
}

bool ButtonState::isHeld() const {
    return _isHeld;
}

bool HAL_isButtonPressed(const Button idx) {
    assert(idx == Button::B1 || idx == Button::B2 || idx == Button::B3);

    #if defined(UNIT_TEST)
        (void)idx;
        return false;
    #else
        switch (idx) {
            case Button::B1:
                return HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_RESET;
            case Button::B2:
                return HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_RESET;
            case Button::B3:
                return HAL_GPIO_ReadPin(BUTTON3_GPIO_Port, BUTTON3_Pin) == GPIO_PIN_RESET;
            default:
                assert(false && "[HAL_isButtonPressed] Invalid button pin ID");
                return false;
        }
    #endif
}

Button pinIdToButton(uint32_t pin_id) {
    switch (pin_id) {
        case BUTTON1_Pin: return Button::B1;
        case BUTTON2_Pin: return Button::B2;
        case BUTTON3_Pin: return Button::B3;
        default:
            assert(false && "[PinIdToButton] Invalid button pin ID");
            return Button::B1;
    }
}