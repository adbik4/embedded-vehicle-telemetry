/*
    This file provides modified or exposed versions of the algobuild library functions
    for use in my own applications
*/ 

#pragma once

#include <stdint.h>

void AlgoBuilder_Init(void);
void Time_Handler(void);
void SensorHub_Init(uint32_t data_rate_control, uint32_t data_rate, uint32_t accel_fs, uint32_t gyro_fs);