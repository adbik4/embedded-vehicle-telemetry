#pragma once

#ifndef UNIT_TEST
#include "FreeRTOS.h"
#include "semphr.h"
#endif

#include "io.hpp"
#include "utilities.hpp"
#include "L76X.h"
#include "motion_fx.h"
#include <cassert>
#include <cstring>

#ifndef UNIT_TEST
    #include "ab_sensor_hub.h"
    extern sensor_hub_data_t sensor_hub_data;
#endif

typedef struct {
    uint8_t enable_6x;
    MFX_output_t sensor_fusion;
    MFX_MagCal_output_t mag_cal;
} MFX_State_t;

typedef struct {
    Date_t date;
    UTC_Time_t time;
    DMS_Coordinates coors; // DMS: Degrees Minutes Seconds
    double speed;   // SOG: Speed Over Ground
} GNSS_State_t;

typedef struct {
    uint32_t timestamp_us;
    float_t angular_rate[3];
    float_t mag_field[3];
    MFX_State_t mfx;
    GNSS_State_t gnss;
} SystemState_t;


#ifndef UNIT_TEST
static StaticSemaphore_t SystemStateMutexBuffer;

class SystemState : public Singleton<SystemState> {
private:
    SemaphoreHandle_t _mutex{xSemaphoreCreateMutexStatic(&SystemStateMutexBuffer)};
    SystemState_t _state;

    SystemState() : _state{} { configASSERT(_mutex != nullptr); };

    void assertAcquired() {
        configASSERT(xSemaphoreGetMutexHolder(_mutex) == xTaskGetCurrentTaskHandle());
    }

    friend class Singleton<SystemState>;

public:
    /* Acquires the System State mutex */
    void acquire() {
        configASSERT(_mutex != nullptr);
        configASSERT(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
    }

    /* Releases the System State mutex */
    void release() {
        assertAcquired();
        configASSERT(xSemaphoreGive(_mutex) == pdTRUE);
    }

    /* Copies the relevant variables from other parts of the system into the SystemState */
    void updateState() {
        assertAcquired();
        _state.timestamp_us = sensor_hub_data.timestamp_us;
        memcpy(_state.angular_rate, sensor_hub_data.angular_rate, sizeof(_state.angular_rate));
        memcpy(_state.mag_field, sensor_hub_data.magnetic_field, sizeof(_state.mag_field));

        _state.mfx.enable_6x = sensor_hub_data.motion_fx_6x_enable;
        if (_state.mfx.enable_6x) {
            _state.mfx.sensor_fusion = sensor_hub_data.mfx_6x;
        } else {
            _state.mfx.sensor_fusion = sensor_hub_data.mfx_9x;
            _state.mfx.mag_cal = sensor_hub_data.mfx_magcal;
        }
    };
    
    /* Overwrites the SystemState with a new_state */
    void setState(const SystemState_t& new_state) {
        assertAcquired();
        _state = new_state;
    }
    
    /* Returns a copy of the System State*/
    SystemState_t getState() {
        assertAcquired();
        return _state;
    }
};

#else

class MockSystemState : public Singleton<MockSystemState> {
public:
    void acquire() {};
    void release() {};
    void updateState() {};
    void setState(const SystemState_t& new_state) {};
    SystemState_t getState() {return {};};
};

#endif