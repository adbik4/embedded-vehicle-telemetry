/**
  ******************************************************************************
  * @file    demo_serial.c
  * @author  MEMS Software Solutions Team
  * @brief   Handler Serial Protocol
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

/* Includes ------------------------------------------------------------------*/
#include "demo_serial.h"
#include "ab_sensor_hub.h"
#include "main.h"
#include "com.h"
#include "sensor_unicleo_id.h"
#include "sensor_commands.h"
#include "bsp_ip_conf.h"

#ifdef CUSTOM_ALGOBUILDER_FW_TEMPLATE
#include "custom_mems_conf_app.h"
#endif

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Communication
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#ifndef ACC_GYR_UNICLEO_ID
#define ACC_GYR_UNICLEO_ID UNKNOWN_UNICLEO_ID
#endif

#ifndef MAG_UNICLEO_ID
#define MAG_UNICLEO_ID UNKNOWN_UNICLEO_ID
#endif

#ifndef HUM_TEMP_UNICLEO_ID
#define HUM_TEMP_UNICLEO_ID UNKNOWN_UNICLEO_ID
#endif

#ifndef PRESS_UNICLEO_ID
#define PRESS_UNICLEO_ID UNKNOWN_UNICLEO_ID
#endif

#if (defined (USE_BLE_OUTPUT))
#define COMMUNICATION "BLE"
#else
#define COMMUNICATION "COM"
#endif /* USE_BLE_OUTPUT */

#if (defined (STM32F4))
#define MCU_FAMILY "STM32F4"
#elif (defined (STM32L4))
#define MCU_FAMILY "STM32L4"
#else
#define MCU_FAMILY ""
#endif /* STM32F4 */

#define INIT_ERROR_QUEUE_SIZE  4U

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint8_t DataStreamingDest = 1;

static uint32_t init_error_queue[INIT_ERROR_QUEUE_SIZE];
static uint32_t init_error_queue_index;

/* Global variables ----------------------------------------------------------*/
offline_data_t offline_data;

extern volatile uint32_t SensorReadRequest;

extern volatile uint32_t Update16Hz;
extern volatile uint32_t Update25Hz;
extern volatile uint32_t Update50Hz;
extern volatile uint32_t Update100Hz;
extern const char Identification_String[];

extern uint8_t PresentationString[];
extern uint8_t PresentationStringLen;

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Build the reply header
  * @param  Msg the pointer to the message to be built
  * @retval None
  */
void BUILD_REPLY_HEADER(TMsg *Msg)
{
  Msg->Data[0] = Msg->Data[1];
  Msg->Data[1] = DEV_ADDR;
  Msg->Data[2] += CMD_Reply_Add;
}

/**
  * @brief  Initialize the streaming header
  * @param  Msg the pointer to the header to be initialized
  * @retval None
  */
void INIT_STREAMING_HEADER(TMsg *Msg)
{
  Msg->Data[0] = DataStreamingDest;
  Msg->Data[1] = DEV_ADDR;
  Msg->Data[2] = CMD_Start_Data_Streaming;
  Msg->Len = 3;
}

/**
  * @brief  Initialize the streaming message
  * @param  Msg the pointer to the message to be initialized
  * @retval None
  */
void INIT_STREAMING_MSG(TMsg *Msg)
{
  int8_t i;

  Msg->Data[0] = DataStreamingDest;
  Msg->Data[1] = DEV_ADDR;
  Msg->Data[2] = CMD_Start_Data_Streaming;
  for (i = 3; i < (STREAMING_MSG_LENGTH + 3); i++)
  {
    Msg->Data[i] = 0;
  }
  Msg->Len = 3;

}

/**
  * @brief  Handle a message
  * @param  Msg the pointer to the message to be handled
  * @retval 1 if the message is correctly handled, 0 otherwise
  */
int32_t HandleMSG(TMsg *Msg)
/*  DestAddr | SouceAddr | CMD | PAYLOAD */
/*      1          1        1       N    */
{
  uint32_t i;
  int32_t ret = 1;
  static uint32_t last_run_16Hz = 0;
  static uint32_t last_run_25Hz = 0;
  static uint32_t last_run_50Hz = 0;
  static uint32_t last_run_100Hz = 0;
  const char Firmware_Info[] = {"@(#),"__DATE__","__TIME__","MCU_FAMILY","COMMUNICATION",@($)"};


  if (Msg->Len < 2U)
  {
    ret = 0;
  }

  if (Msg->Data[0] != DEV_ADDR)
  {
    ret = 0;
  }

  if (ret != 0)
  {
    switch (Msg->Data[2])   /* CMD */
    {
      case CMD_Ping:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Read_PresString:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          i = 0;
          while (i < ((uint32_t)PresentationStringLen - 1U))
          {
            Msg->Data[3U + i] = PresentationString[i];
            i++;
          }

          Msg->Len = 3U + i;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_PRESSURE_Init:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Serialize_s32(&Msg->Data[3], PRESS_UNICLEO_ID, 4);
          Msg->Len = 3U + 4U;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_HUMIDITY_TEMPERATURE_Init:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Serialize_s32(&Msg->Data[3], HUM_TEMP_UNICLEO_ID, 4);
          Msg->Len = 3U + 4U;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_ACCELERO_GYRO_Init:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Serialize_s32(&Msg->Data[3], ACC_GYR_UNICLEO_ID, 4);
          Msg->Len = 3U + 4U;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_MAGNETO_Init:
        if (Msg->Len != 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Serialize_s32(&Msg->Data[3], MAG_UNICLEO_ID, 4);
          Msg->Len = 3U + 4U;
          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Start_Data_Streaming:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          if (sensor_hub_data.data_rate_control == DRC_TIMER)
          {
            TIM_AB_Start();
          }
          TIM_AL_Start();
          DataLoggerActive = 1;

          DataStreamingDest = Msg->Data[1];
          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;

          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Stop_Data_Streaming:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          DataLoggerActive = 0;
          if (sensor_hub_data.data_rate_control == DRC_TIMER)
          {
            TIM_AB_Stop();
          }
          TIM_AL_Stop();

          BUILD_REPLY_HEADER(Msg);

          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Set_DateTime:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;
          RTC_TimeRegulate(Msg->Data[3], Msg->Data[4], Msg->Data[5]);

          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Sensor:
        /* Check if the command length is at least 5 bytes */
        if (Msg->Len < 5U)
        {
          ret = 0;
        }
        else
        {
          (void)Handle_Sensor_command(Msg);
          ret = 1;
        }
        break;

      case CMD_Offline_Data:
        if (Msg->Len != 57U)
        {
          ret = 0;
        }
        else
        {
          offline_data.timestamp_us = 0;
          (void)memcpy(&offline_data.timestamp_us, &Msg->Data[3], 6);

          (void)memcpy(&offline_data.pressure, &Msg->Data[9], 4);
          (void)memcpy(&offline_data.temperature, &Msg->Data[13], 4);
          (void)memcpy(&offline_data.humidity, &Msg->Data[17], 4);

          (void)memcpy(&offline_data.acceleration_x_mg, &Msg->Data[21], 4);
          (void)memcpy(&offline_data.acceleration_y_mg, &Msg->Data[25], 4);
          (void)memcpy(&offline_data.acceleration_z_mg, &Msg->Data[29], 4);

          (void)memcpy(&offline_data.angular_rate_x_mdps, &Msg->Data[33], 4);
          (void)memcpy(&offline_data.angular_rate_y_mdps, &Msg->Data[37], 4);
          (void)memcpy(&offline_data.angular_rate_z_mdps, &Msg->Data[41], 4);

          (void)memcpy(&offline_data.magnetic_field_x_mgauss, &Msg->Data[45], 4);
          (void)memcpy(&offline_data.magnetic_field_y_mgauss, &Msg->Data[49], 4);
          (void)memcpy(&offline_data.magnetic_field_z_mgauss, &Msg->Data[53], 4);

          offline_data.timestamp_ms += 1000U / sensor_hub_data.data_rate_Hz;

          if ((offline_data.timestamp_ms - last_run_16Hz) >= 62U) /* 16Hz */
          {
            Update16Hz = 1;
            last_run_16Hz = offline_data.timestamp_ms;
          }

          if ((offline_data.timestamp_ms - last_run_25Hz) >= 40U) /* 25Hz */
          {
            Update25Hz = 1;
            last_run_25Hz = offline_data.timestamp_ms;
          }

          if ((offline_data.timestamp_ms - last_run_50Hz) >= 20U) /* 50Hz */
          {
            Update50Hz = 1;
            last_run_50Hz = offline_data.timestamp_ms;
          }

          if ((offline_data.timestamp_ms - last_run_100Hz) >= 10U) /* 100Hz */
          {
            Update100Hz = 1;
            last_run_100Hz = offline_data.timestamp_ms;
          }

          DataLoggerActive = 1;
          SensorReadRequest = 1;
          ret = 1;
        }
        break;

      case CMD_GetFW_Info:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;

          (void)memcpy(&Msg->Data[3], &sensor_hub_data.data_rate_Hz, 4);
          Msg->Len += 4U;

          i = 0;
          while ((uint8_t)Identification_String[i] != 0U)
          {
            Msg->Data[7U + i] = (uint8_t)Identification_String[i];
            i++;
          }
          Msg->Data[7U + i] = 0;
          i++;
          Msg->Len += i;

          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Report_Error:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          uint32_t count;
          uint32_t *p_uint32;

          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;

          p_uint32 = (uint32_t *)(&(Msg->Data[7]));
          Get_Init_Error(&count, p_uint32);
          p_uint32 = (uint32_t *)(&(Msg->Data[3]));
          *p_uint32 = count;
          Msg->Len += (count + 1U) * 4U;

          SendMsg(Msg);
          ret = 1;
        }
        break;

      case CMD_Get_Bin_Info:
        if (Msg->Len < 3U)
        {
          ret = 0;
        }
        else
        {
          BUILD_REPLY_HEADER(Msg);
          Msg->Len = 3;

          i = 0;
          while ((uint8_t)Firmware_Info[i] != 0U)
          {
            Msg->Data[3U + i] = (uint8_t)Firmware_Info[i];
            i++;
          }
          Msg->Data[3U + i] = 0;
          i++;
          Msg->Len += i;

          SendMsg(Msg);
          ret = 1;
        }
        break;

      default:
        ret = 0;
        break;
    }
  }

  return ret;
}

/**
  * @brief  This function initialize the error queue
  * @param  None
  * @retval None
  */
void Init_Error(void)
{
  init_error_queue_index = 0;
  (void)memset(init_error_queue, 0, INIT_ERROR_QUEUE_SIZE * sizeof(uint32_t));
}

/**
  * @brief  This function reports errors to application
  * @param  err - reported error code
  * @retval None
  */
void Report_Error(uint32_t err)
{
  TMsg msg;
  uint32_t err_num = 1;

  msg.Data[0] = 1;
  msg.Data[1] = DEV_ADDR;
  msg.Data[2] = (uint8_t)CMD_Report_Error + CMD_Reply_Add;
  (void)memcpy(&(msg.Data[3]), &err_num, sizeof(uint32_t));
  (void)memcpy(&(msg.Data[7]), &err, sizeof(uint32_t));
  msg.Len = 11;

  SendMsg(&msg);
}

/**
  * @brief  This function will put initialization errors into queue to not disappear,
  *         because they occurred before connection was established.
  *         Application can read these errors using CMD_Report_Error command.
  * @param  err - reported error code
  * @retval None
  */
void Report_Init_Error(uint32_t err)
{
  if (init_error_queue_index < INIT_ERROR_QUEUE_SIZE)
  {
    init_error_queue[init_error_queue_index] = err;
    init_error_queue_index++;
  }
  else
  {
    init_error_queue[INIT_ERROR_QUEUE_SIZE - 1U] |= AB_ERROR_FLAG_OVERRUN;
  }
}

/**
  * @brief  This function will put initialization errors into buffer.
  * @param  count  - will receive number of errors in buffer
  *         buffer - will receive error list
  * @retval None
  */
void Get_Init_Error(uint32_t *count, uint32_t *buffer)
{
  *count = init_error_queue_index;
  uint32_t i;

  for (i = 0; i < init_error_queue_index; i++)
  {
    buffer[i] = init_error_queue[i];
  }
}

/**
  * @}
  */

/**
  * @}
  */
