/**
  ******************************************************************************
  * @file    ab_user_input.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder input function block control
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
#include "ab_user_input.h"
#include "main.h"
#include "ab_display.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Inputs
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
  * @brief  Inputs function block update
  */
void Input_Value_Init(void *source, sDISPLAY_INFO *header)
{
  header->p_node = source;
}

/**
  * @}
  */

/**
  * @}
  */
