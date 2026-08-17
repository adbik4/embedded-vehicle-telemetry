/**
  ******************************************************************************
  * @file    app_algobuild.h
  * @author  MEMS Software Solutions Team
  * @brief   This file provides code for the configuration of the STMicroelectronics.X-CUBE-ALGOBUILD.1.5.0 instances.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_ALGOBUILD_H
#define APP_ALGOBUILD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "custom_mems_control.h"
#include "custom_mems_control_ex.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Errors reported to application */
#define AB_ERROR_NONE                   0U
#define AB_ERROR_ODR_CHANGED            0x00000001U
#define AB_ERROR_FS_CHANGED             0x00000002U

#define AB_ERROR_FLAG_OVERRUN           0x80000000U

#define AB_ERROR_CATEGORY_ERROR         0x08000000U
#define AB_ERROR_CATEGORY_WARNING       0x04000000U
#define AB_ERROR_CATEGORY_LOG           0x02000000U
#define AB_ERROR_CATEGORY_DEBUG         0x01000000U

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void MX_Library_Init(void);
void MX_Library_Process(void);

void TIM_AB_Config(uint32_t freq);
void TIM_AB_Start(void);
void TIM_AB_Stop(void);

void TIM_AL_Start(void);
void TIM_AL_Stop(void);

uint64_t DWT_GetTickUS(void);

void RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_ALGOBUILD_H */
