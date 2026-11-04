#pragma once

#if defined(UNIT_TEST)
    #include "L76X.h"

struct sensor_hub_data_t {};
#else
    #include "L76X.h"
    #include "ab_sensor_hub.h"
#endif

#include "io.hpp"
#include "utilities.hpp"

extern sensor_hub_data_t sensor_hub_data;

class  StateIf {
public:
    virtual ~StateIf() {};

    /* read-only */
    virtual const sensor_hub_data_t& getIMUdata() const = 0;

    /* read-write */
    virtual GPSdata_t& getGNSSdata() = 0;
    virtual const GPSdata_t& getGNSSdata() const = 0;
};

class SystemState : public Singleton<SystemState>, public StateIf {
private:
    sensor_hub_data_t& imu_data;
    GPSdata_t gnss_data;
    
    // TODO: redo button state handling (remove from here)
    Button button1;
    Button button2;
    Button button3;

    friend void StartInputManagerTask(void *argument);

public:
    SystemState(const SystemState&) = delete;
    SystemState& operator=(const SystemState&) = delete;
    SystemState() : imu_data(sensor_hub_data), gnss_data{} {}

    /* read-only */
    const sensor_hub_data_t& getIMUdata() const override;

    /* read-write */
    GPSdata_t& getGNSSdata() override;
    const GPSdata_t& getGNSSdata() const override;
};