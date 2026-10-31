#pragma once

#include "L76X.h"
#include "ab_sensor_hub.h"
#include "io.hpp"
#include "utilities.hpp"
#include <optional>

extern sensor_hub_data_t sensor_hub_data;

class SystemState {
private:
    sensor_hub_data_t& imu_data;
    GPSdata_t gnss_data;
    
    ButtonState button1;
    ButtonState button2;
    ButtonState button3;

    friend void StartInputManagerTask(void *argument);

public:
    SystemState(const SystemState&) = delete;
    SystemState& operator=(const SystemState&) = delete;
    SystemState() : imu_data(sensor_hub_data), gnss_data{} {}

    /* read-only */
    const sensor_hub_data_t& getIMUdata() const;
    std::optional<const ButtonState> getButtonState(const Button id) const;

    /* read-write */
    GPSdata_t& getGNSSdata();
    const GPSdata_t& getGNSSdata() const;
};