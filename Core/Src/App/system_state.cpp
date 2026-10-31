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