#include "io.hpp"

#include <cassert>
#include <cstdint>

#if !defined(UNIT_TEST)
    #include "main.h"
    #include "stm32u3xx_hal_gpio.h"
#endif

/* Returns the current state */
const ButtonState Button::getState() const {
    return curr_state;
}

/* Advances to the next state */
void Button::advanceState() {
    curr_state = transitions.at((static_cast<size_t>(curr_state) + 1) % transitions.size());
}

/* Resets the current state to idle */
void Button::goIdle() {
    curr_state = ButtonState::idle;
}

bool HAL_isButtonPressed(const ButtonId idx) {
    assert(idx == ButtonId::B1 || idx == ButtonId::B2 || idx == ButtonId::B3);

    #if defined(UNIT_TEST)
        (void)idx;
        return false;
    #else
        switch (idx) {
            case ButtonId::B1:
                return HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_RESET;
            case ButtonId::B2:
                return HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_RESET;
            case ButtonId::B3:
                return HAL_GPIO_ReadPin(BUTTON3_GPIO_Port, BUTTON3_Pin) == GPIO_PIN_RESET;
            default:
                assert(false && "[HAL_isButtonPressed] Invalid button pin ID");
                return false;
        }
    #endif
}

ButtonId pinIdToButton(uint32_t pin_id) {
    switch (pin_id) {
        case BUTTON1_Pin: return ButtonId::B1;
        case BUTTON2_Pin: return ButtonId::B2;
        case BUTTON3_Pin: return ButtonId::B3;
        default:
            assert(false && "[PinIdToButton] Invalid button pin ID");
            return ButtonId::B1;
    }
}