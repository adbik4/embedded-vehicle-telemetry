/**
  ******************************************************************************
  * @file    ab_fff.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_user_input.c
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
#ifndef FFT_H
#define FFT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ab_math.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FFT_Init(arm_rfft_fast_instance_f32 *fft_instance, uint16_t length);
void FFT_Calc(arm_rfft_fast_instance_f32 *fft_instance, float_t *in, float_t *out, int32_t *size, int32_t *full,
              float_t *input_buffer, int32_t *buffer_index, uint32_t window, uint16_t length);
void FFT_Peak(float32_t *in, float_t *amplitude, float_t *frequency, uint16_t length, uint8_t exclude_dc);

#ifdef __cplusplus
}
#endif

#endif /* FFT_H */
