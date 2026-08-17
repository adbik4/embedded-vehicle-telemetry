/**
  *******************************************************************************
  * @file    algo_builder.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for algo_builder.c
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
#ifndef ALGO_BUILDER_H
#define ALGO_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include "main.h"
#include "serial_protocol.h"
#include "sensor_commands.h"
#include "ab_math.h"
#include "ab_buffers.h"
#include "ab_display.h"
#include "ab_fft.h"
#include "ab_sensor_hub.h"
#include "ab_sequential_logic.h"
#include "ab_signal.h"
#include "ab_user_input.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MODULO %

/* External variables --------------------------------------------------------*/
extern volatile int32_t Message_Length;
extern sDISPLAY_INFO display_info_list[];
extern const char Identification_String[];

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void AB_Init(void);
void AB_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* ALGO_BUILDER_H */
