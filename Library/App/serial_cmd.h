/**
  *******************************************************************************
  * @file    serial_cmd.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for serial_cmd.c
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
#ifndef SERIAL_CMD_H
#define SERIAL_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* GENERIC  CMD  (0x00 - 0x0F) -------------------*/
#define CMD_Ping                       0x01
#define CMD_Read_PresString            0x02
#define CMD_Get_Streaming_Status       0x03
#define CMD_Report_Error               0x04
#define CMD_UploadXX                   0x05
#define CMD_ChangeSF                   0x07
#define CMD_Start_Data_Streaming       0x08
#define CMD_Stop_Data_Streaming        0x09
#define CMD_Start_Data_Sending         0x0A
#define CMD_Stop_Data_Sending          0x0B
#define CMD_Offline_Data               0x10
#define CMD_GetFW_Info                 0x11
#define CMD_Get_Bin_Info               0x13

#define CMD_Set_DateTime               0x0C
#define CMD_Enter_DFU_Mode             0x0E
#define CMD_Reset                      0x0F
#define CMD_Reply_Add                  0x80U

#define CMD_Sensor                     0x50

/* ENVIRONMENTAL  CMD  (0x60 - 0x6F) -------------*/
#define CMD_PRESSURE_Init              0x60
#define CMD_HUMIDITY_TEMPERATURE_Init  0x62

/* INERTIAL  CMD  (0x70 - 0x7F) ------------------*/
#define CMD_ACCELERO_GYRO_Init         0x76
#define CMD_MAGNETO_Init               0x7A

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_CMD_H */
