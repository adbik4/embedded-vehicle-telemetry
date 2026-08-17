/**
  ******************************************************************************
  * @file    ab_sequential_logic.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder sequential logic function blocks
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
#include "ab_sequential_logic.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Sequential_Logic
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
  * @brief  R-S Flip Flop
  */
void RSFlipFlop(const int32_t *set, const int32_t *reset, int32_t *out, int32_t *outn, int32_t *mem)
{
  int32_t status = reset[0] + (2 * set[0]);

  switch (status)
  {
    case 0:
      out[0] = mem[0];
      break;
    case 1:
      out[0] = 0;
      break;
    case 2:
      out[0] = 1;
      break;
    default:
      out[0] = mem[0];
      break;
  }

  if (out[0] == 0)
  {
    outn[0] = 1;
  }
  else
  {
    outn[0] = 0;
  }
  mem[0] = out[0];
}

/**
  * @}
  */

/**
  * @}
  */
