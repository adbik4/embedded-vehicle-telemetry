/**
  *******************************************************************************
  * @file    demo_serial.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for demo_serial.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEMO_SERIAL_H
#define DEMO_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "serial_protocol.h"
#include "serial_cmd.h"
#include <math.h>

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint64_t timestamp_us;
  uint32_t timestamp_ms;
  float_t pressure;
  float_t humidity;
  float_t temperature;
  int32_t acceleration_x_mg;
  int32_t acceleration_y_mg;
  int32_t acceleration_z_mg;
  int32_t angular_rate_x_mdps;
  int32_t angular_rate_y_mdps;
  int32_t angular_rate_z_mdps;
  int32_t magnetic_field_x_mgauss;
  int32_t magnetic_field_y_mgauss;
  int32_t magnetic_field_z_mgauss;
} offline_data_t;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define SENDER_UART  0x01
#define SENDER_USB   0x02
#define SENDER_SPI   0x03

#define DEV_ADDR                            ((uint8_t)50)
#define I2C_DATA_MAX_LENGTH_BYTES           16
#define STREAMING_MSG_LENGTH                122

/* Exported variables --------------------------------------------------------*/
extern uint8_t DataLoggerActive;
extern uint32_t Sensors_Enabled;
extern TIM_HandleTypeDef AB_TimHandle;
extern TIM_HandleTypeDef AL_TimHandle;

extern UART_HandleTypeDef UartHandle;

extern offline_data_t offline_data;

/* Exported macros -----------------------------------------------------------*/
/*#define MIN(A,B) ((A)<(B)?(A):(B)) */

/* Exported functions ------------------------------------------------------- */
int32_t HandleMSG(TMsg *Msg);
void BUILD_REPLY_HEADER(TMsg *Msg);
void INIT_STREAMING_MSG(TMsg *Msg);
void BUILD_NACK_HEADER(TMsg *Msg);
void INIT_STREAMING_HEADER(TMsg *Msg);

extern void Init_Error(void);
extern void Report_Error(uint32_t err);
extern void Report_Init_Error(uint32_t err);
extern void Get_Init_Error(uint32_t *count, uint32_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_SERIAL_H */
