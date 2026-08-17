/**
  ******************************************************************************
  * @file    sensor_commands.c
  * @author  MEMS Software Solutions Team
  * @brief   Handle commands for sensor
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
#include "sensor_commands.h"
#include "demo_serial.h"
#include "com.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Configuration
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern sDISPLAY_INFO display_info_list[];

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Handle Sensors command
  * @param  Msg the pointer to the message to be handled
  * @param  custom_values the pointer to the custom values
  * @retval 1 if the message is correctly handled, 0 otherwise
  */
int32_t Handle_Sensor_command(TMsg *Msg)
{
  int32_t ret;

  /* Commands */
  switch (Msg->Data[3])
  {
    case SC_GET_CONFIG_STRING:
      ret = SC_Get_Config_String(Msg, Msg->Data[4]);
      break;

    case SC_GET_CUSTOM_CONFIG:
      ret = SC_Get_Custom_Config(Msg);
      break;

    case SC_SET_CUSTOM_VALUES:
      ret = SC_Set_Custom_Values(Msg);
      break;

    case SC_GET_CUSTOM_VALUES:
      ret = SC_Get_Custom_Values(Msg);
      break;

    default:
      ret = 0;
      break;
  }

  return ret;
}

int32_t SC_Get_Custom_Config(TMsg *Msg)
{
  int32_t i;
  int32_t j;
  int32_t conn_index;
  int32_t index;
  sCONFIG_RECORD_t *cr;

  BUILD_REPLY_HEADER(Msg);

  Msg->Data[5] = 0; /* number of records */
  index = 1;
  i = 0;

  while (display_info_list[i].info_index != 0U)
  {
    display_info_list[i].already_processed = 0;
    i++;
  }
  i = 0;
  while (display_info_list[i].info_index != 0U)
  {
    if (display_info_list[i].info_type > (uint8_t)INFO_TYPE_AFLOAT) /* output configurations only */
    {
      if (display_info_list[i].already_processed == 0U)
      {
        cr = (sCONFIG_RECORD_t *) & (Msg->Data[5 + index]);
        display_info_list[i].already_processed = 1;  /* mark as already processed */
        cr->var_count = (uint8_t)display_info_list[i].variable_count;
        cr->var_type = display_info_list[i].variable_type;
        conn_index = 0;
        cr->conn[conn_index] = display_info_list[i].info_index;
        conn_index++;
        cr->conn_size = 1;
        Msg->Data[5]++; /* number of records */
        if (display_info_list[i].info_type != (uint8_t)INFO_TYPE_FFT)
        {
          j = 0;
          while (display_info_list[j].info_index != 0U)
          {
            if (display_info_list[j].info_type > (uint8_t)INFO_TYPE_AFLOAT) /* output configurations only */
            {
              if (display_info_list[j].already_processed == 0U)
              {
                if ((display_info_list[j].stream_position == display_info_list[i].stream_position) &&
                    (display_info_list[j].info_type != (uint8_t)INFO_TYPE_FFT)) /* add other outputs with the same stream addr. */
                {
                  cr->conn[conn_index] = display_info_list[j].info_index;
                  conn_index++;
                  cr->conn_size++;
                  display_info_list[j].already_processed = 1;  /* mark as already processed */
                }
              }
            }
            j++;
          }
        }
        index += 3 + conn_index;  /* add connections count */
      }
    }
    i++;
  }
  /* input configurations */
  i = 0;
  while (display_info_list[i].info_index != 0U)
  {
    if (display_info_list[i].info_type <= (uint8_t)INFO_TYPE_AFLOAT) /* input configurations only */
    {
      if (display_info_list[i].already_processed == 0U)
      {
        cr = (sCONFIG_RECORD_t *) & (Msg->Data[5 + index]);
        display_info_list[i].already_processed = 1;  /* mark as already processed */
        cr->var_count = (uint8_t)display_info_list[i].variable_count;
        cr->var_type = display_info_list[i].variable_type;
        cr->var_type += (uint8_t)VAR_TYPE_INPUT;
        cr->conn[0] = display_info_list[i].info_index;
        cr->conn_size = 1;
        Msg->Data[5]++; /* number of records */
        index += 4;
      }
    }
    i++;
  }

  Msg->Len = 5U + (uint32_t)index;

  SendMsg(Msg);

  return 1;
}

int32_t SC_Get_Config_String(TMsg *Msg, uint8_t id)
{
  int32_t i = 0;
  uint32_t j = 0;
  BUILD_REPLY_HEADER(Msg);

  while (display_info_list[i].info_index != 0U)
  {
    if (display_info_list[i].info_index == id)
    {
      for (j = 0; j < strlen(display_info_list[i].config_string); j++)
      {
        Msg->Data[5 + j] = (uint8_t)display_info_list[i].config_string[j];
      }
    }
    i++;
  }

  Msg->Len = 5U + j;

  SendMsg(Msg);

  return 1;
}

int32_t SC_Set_Custom_Values(TMsg *Msg)
{
  int32_t i = 0;
  while (display_info_list[i].info_index != 0U)
  {
    if (display_info_list[i].info_index == Msg->Data[4])
    {
      if (display_info_list[i].info_type == (uint8_t)INFO_TYPE_ABITS)
      {
        if (display_info_list[i].p_node != 0)
        {
          for (uint32_t j = 0; j < display_info_list[i].variable_count; j++)
          {
            ((uint32_t *)(display_info_list[i].p_node))[j] = ((Msg->Data[5] & (1U << j)) != 0U) ? 1U : 0U;
          }
        }
      }
      if (display_info_list[i].info_type == (uint8_t)INFO_TYPE_AINT32)
      {
        if (display_info_list[i].p_node != 0)
        {
          (void)memcpy(display_info_list[i].p_node, Msg->Data + 5, (uint32_t)display_info_list[i].variable_count * 4U);
        }
      }
      if (display_info_list[i].info_type == (uint8_t)INFO_TYPE_AFLOAT)
      {
        if (display_info_list[i].p_node != 0)
        {
          (void)memcpy(display_info_list[i].p_node, Msg->Data + 5, (uint32_t)display_info_list[i].variable_count * 4U);
        }
      }
    }
    i++;
  }

  BUILD_REPLY_HEADER(Msg);
  Msg->Len = 5;

  SendMsg(Msg);

  return 1;
}

int32_t SC_Get_Custom_Values(TMsg *Msg)
{
  int32_t i = 0;
  uint32_t j;
  uint8_t break_while_loop = 0;

  while (display_info_list[i].info_index != 0U)
  {
    if (display_info_list[i].info_index == Msg->Data[4])
    {
      switch (display_info_list[i].info_type)
      {
        case INFO_TYPE_ABITS:
          if (display_info_list[i].p_node != 0)
          {
            Msg->Data[5] = 0;
            for (j = 0; j < display_info_list[i].variable_count; j++)
            {
              Msg->Data[5] |= (((uint32_t *)(display_info_list[i].p_node))[j] != 0U) ? (1U << j) : 0U;
            }
          }
          Msg->Len = 6;
          break_while_loop = 1;
          break;

        case INFO_TYPE_AINT32:
          if (display_info_list[i].p_node != 0)
          {
            (void)memcpy(Msg->Data + 5, display_info_list[i].p_node, (uint32_t)display_info_list[i].variable_count * 4U);
          }
          Msg->Len = 5U + ((uint32_t)display_info_list[i].variable_count * 4U);
          break_while_loop = 1;
          break;

        case INFO_TYPE_AFLOAT:
          if (display_info_list[i].p_node != 0)
          {
            (void)memcpy(Msg->Data + 5, display_info_list[i].p_node, (uint32_t)display_info_list[i].variable_count * 4U);
          }
          Msg->Len = 5U + ((uint32_t)display_info_list[i].variable_count * 4U);
          break_while_loop = 1;
          break;

        default:
          break;
      }
      if (break_while_loop != 0U)
      {
        break;
      }
    }
    i++;
  }

  BUILD_REPLY_HEADER(Msg);

  SendMsg(Msg);

  return 1;
}

/**
  * @}
  */

/**
  * @}
  */
