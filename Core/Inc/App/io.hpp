#pragma once

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
    held
};

static const std::array<ButtonState, 4> transitions = {
    ButtonState::idle,
    ButtonState::debouncing,
    ButtonState::pressed,
    ButtonState::held
};

class Button {
protected:
    ButtonState curr_state;

public:
    Button() : curr_state(ButtonState::idle) {};

    const ButtonState getState() const;
    void advanceState();
    void goIdle();
};

#ifndef UNIT_TEST
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