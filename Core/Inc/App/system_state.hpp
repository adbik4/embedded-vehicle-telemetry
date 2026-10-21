#pragma once

#include "L76X.h"
#include "ab_sensor_hub.h"

extern sensor_hub_data_t sensor_hub_data;

class SystemState {
private:
    sensor_hub_data_t& imu_data;
    GPSdata_t gnss_data;

public:
    SystemState(const SystemState&) = delete;
    SystemState& operator=(const SystemState&) = delete;
    SystemState() : imu_data(sensor_hub_data), gnss_data{} {}

    /* read-only */
    const sensor_hub_data_t& getIMUdata() const { return imu_data; } 

    /* read-write */
    GPSdata_t& getGNSSdata() { return gnss_data; } 
    const GPSdata_t& getGNSSdata() const { return gnss_data; }
};