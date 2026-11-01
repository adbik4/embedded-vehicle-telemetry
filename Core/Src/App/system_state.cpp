#include "system_state.hpp"

#include <cassert>

const sensor_hub_data_t& SystemState::getIMUdata() const {
    return imu_data;
}

GPSdata_t& SystemState::getGNSSdata() {
    return gnss_data;
}

const GPSdata_t& SystemState::getGNSSdata() const {
    return gnss_data;
}

const ButtonState& SystemState::getButtonState(const Button idx) const {
    assert(idx == Button::B1 || idx == Button::B2 || idx == Button::B3);

    switch(idx) {
        case Button::B1:
            return button1;
            break;

        case Button::B2:
            return button2;
            break;

        case Button::B3:
            return button3;
            break;
        
        default:
            return button1;
            break;
    }
}

void SystemState::setButtonState(const Button idx, const bool isPressed, const bool isHeld) {
    assert(idx == Button::B1 || idx == Button::B2 || idx == Button::B3);
    
    switch(idx) {
        case Button::B1:
            button1._isPressed = isPressed;
            button1._isHeld = isHeld;
            break;
        case Button::B2:
            button2._isPressed = isPressed;
            button2._isHeld = isHeld;
            break;
        case Button::B3:
            button3._isPressed = isPressed;
            button3._isHeld = isHeld;
            break;
    }
}

void SystemState::clearButtonState(const Button idx) {
    setButtonState(idx, false, false);
}