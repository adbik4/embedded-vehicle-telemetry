/**
  ******************************************************************************
  * @file    ab_display.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder display function block control
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
#include "ab_display.h"
#include "main.h"
#include "serial_protocol.h"
#include "com.h"
#include <math.h>

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Display
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern TMsg MsgDat;

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Display function block update
  */
void Display_Update(void *source, sDISPLAY_INFO *header)
{
  if (header->info_type == (uint8_t)INFO_TYPE_FFT)
  {
    SendFFT((float_t *)source, (uint32_t)header->variable_count / 2U, header->info_index);
  }
  else
  {
    if (header->variable_type == (uint8_t)VAR_TYPE_BIT)
    {
      if (header->variable_count > 8U)
      {
        uint16_t tmp = 0;
        uint8_t i;
        for (i = 0; i < header->variable_count; i++)
        {
          if (((uint32_t *)(source))[i] != 0U)
          {
            tmp += ((uint16_t)1 << (uint16_t)i);
          }
        }
        SerializeToMsg((uint8_t)STREAM_DATA + (uint8_t)header->stream_position, &tmp, 2);
      }
      else
      {
        uint8_t tmp = 0;
        uint8_t i;
        for (i = 0; i < header->variable_count; i++)
        {
          if (((uint32_t *)source)[i] != 0U)
          {
            tmp += (1U << i);
          }
        }
        SerializeToMsg((uint8_t)STREAM_DATA + (uint8_t)header->stream_position, &tmp, 1);
      }
    }
    else
    {
      SerializeToMsg((uint8_t)STREAM_DATA + (uint8_t)header->stream_position, source, (uint32_t)header->variable_count * 4U);
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
