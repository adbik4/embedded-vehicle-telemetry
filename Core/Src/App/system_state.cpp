#include "system_state.hpp"

const sensor_hub_data_t& SystemState::getIMUdata() const {
    return imu_data;
}

GPSdata_t& SystemState::getGNSSdata() {
    return gnss_data;
}

const GPSdata_t& SystemState::getGNSSdata() const {
    return gnss_data;
}

std::optional<const ButtonState> SystemState::getButtonState(const Button id) const {
    switch(id) {
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
            PRINT_DEBUG("[SystemState::GetButtonState] Unknown Button ID: %d", static_cast<int>(id));
            return std::nullopt;
            break;
    }
}