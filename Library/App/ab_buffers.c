/**
  ******************************************************************************
  * @file    ab_buffers.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder buffer function blocks
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
#include "ab_buffers.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Buffers
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Stores data into linear buffer
  */
void LinearBuffer(float_t *in, float_t *out, int32_t *size, int32_t *full, int32_t *mem, int32_t buffer_size)
{
  *size = buffer_size;
  out[*mem] = in[0];

  if (*mem == (buffer_size - 1))
  {
    *mem = 0;
    *full = 1;
  }
  else
  {
    *mem = *mem + 1;
    *full = 0;
  }
}

/**
  * @brief  Stores data into circular buffer
  */
void CircularBuffer(float_t *in, float_t *out, int32_t *size, int32_t *full, int32_t *mem, int32_t buffer_size)
{
  *size = buffer_size;
  out[*mem] = in[0];
  *mem = (*mem + 1) % buffer_size;

  if (*mem == (buffer_size - 1))
  {
    *full = 1;
  }
}

/**
  * @brief  Extract part of data from input buffer
  */
void SubBuffer(float_t *in, int32_t in_size, float_t *out, int32_t out_size, int32_t start)
{
  if ((start + out_size) > in_size) { return; }

    for (int32_t i = start; i < (start + out_size); i++)
    {
      out[i - start] = in[i];
    }
}

/**
  * @}
  */

/**
  * @}
  */
