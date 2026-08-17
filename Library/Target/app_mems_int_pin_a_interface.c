/**
  ******************************************************************************
  * @file    app_mems_int_pin_a_interface.c
  * @author  MEMS Software Solutions Team
  * @brief   This file contains the MEMS INT pin A interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_mems_int_pin_a_interface.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Int_Pin_Interface
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
EXTI_HandleTypeDef hexti1 = {.Line = EXTI_LINE_1};

/* Global variables ----------------------------------------------------------*/
extern volatile uint8_t SensorReadRequest;

/* Private function prototypes -----------------------------------------------*/
static void mems_int_pin_a_hardware_event_isr(void);

/* Functions Definition ------------------------------------------------------*/
void set_mems_int_pin_a_exti(void)
{
  /* register event irq handler */
  HAL_EXTI_GetHandle(&hexti1, EXTI_LINE_1);
  HAL_EXTI_RegisterCallback(&hexti1, HAL_EXTI_COMMON_CB_ID, mems_int_pin_a_hardware_event_isr);
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

static void mems_int_pin_a_hardware_event_isr(void)
{
  SensorReadRequest = 1;
}

/**
  * @}
  */

/**
  * @}
  */
