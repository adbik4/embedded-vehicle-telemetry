#pragma once

#if defined(UNIT_TEST)
using TickType_t = unsigned long;
#else
#include "FreeRTOS.h"
#endif
#include <array>
#include <cstdint>

enum class ButtonId {
    B1 = 0,
    B2,
    B3
};

enum class ButtonState {
    idle = 0,
    debouncing,
    pressed,
};

static const std::array<ButtonState, 4> transitions = {
    ButtonState::idle,
    ButtonState::debouncing,
    ButtonState::pressed,
};

class Button {
protected:
    ButtonState curr_state;
    TickType_t _tickRef;

    friend void handleButtonEvent(Button& button, bool* actionDetected);

public:
    Button() : curr_state(ButtonState::idle), _tickRef(0) {};

    const ButtonState getState() const;
    void advanceState();
    void goIdle();
};

#ifndef UNIT_TEST
    void handleButtonEvent(Button& button, bool* actionDetected);
    bool HAL_isButtonPressed(const ButtonId idx);
    ButtonId pinIdToButton(uint32_t pin_id);
#else
    class ButtonMock : public Button {
    public:
        void setState(const ButtonState state) {
            curr_state = state;
        }
    };
#endif