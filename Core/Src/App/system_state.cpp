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