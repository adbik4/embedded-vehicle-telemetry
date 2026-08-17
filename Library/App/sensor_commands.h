/**
  *******************************************************************************
  * @file    sensor_commands.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for sensor_commands.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef SENSOR_COMMANDS
#define SENSOR_COMMANDS

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "serial_protocol.h"
#include "main.h"
#include "ab_display.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SC_ACCELEROMETER                                                0x01
#define SC_GYROSCOPE                                                    0x02
#define SC_MAGNETOMETER                                                 0x03
#define SC_TEMPERATURE                                                  0x04
#define SC_HUMIDITY                                                     0x05
#define SC_PRESSURE                                                     0x06
#define SC_UV                                                           0x07
#define SC_SEND_DEFAULTS                                                0xFF

#define SC_GET_SENSOR_NAME                                              0x01
#define SC_READ_REGISTER                                                0x02
#define SC_WRITE_REGISTER                                               0x03
#define SC_GET_FULL_SCALE_LIST                                          0x04
#define SC_SET_FULL_SCALE                                               0x05
#define SC_GET_ODR_LIST                                                 0x06
#define SC_SET_ODR                                                      0x07
#define SC_GET_CUSTOM_NAMES                                             0x08
#define SC_GET_CUSTOM_CONFIG                                            0x09
#define SC_GET_CUSTOM_NAMES2                                            0x0A
#define SC_GET_CUSTOM_NAMES3                                            0x0B
#define SC_SET_CUSTOM_VALUES                                            0x0C
#define SC_GET_CONFIG_STRING                                            0x19
#define SC_GET_CUSTOM_VALUES                                            0x1A

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t Handle_Sensor_command(TMsg *Msg);

int32_t SC_Get_Custom_Config(TMsg *Msg);
int32_t SC_Get_Config_String(TMsg *Msg, uint8_t id);
int32_t SC_Set_Custom_Values(TMsg *Msg);
int32_t SC_Get_Custom_Values(TMsg *Msg);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_COMMANDS */
