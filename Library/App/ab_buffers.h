/**
  ******************************************************************************
  * @file    ab_buffers.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_buffers.c
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
#ifndef BUFFERS_H
#define BUFFERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LinearBuffer(float_t *in, float_t *out, int32_t *size, int32_t *full, int32_t *mem, int32_t buffer_size);
void CircularBuffer(float_t *in, float_t *out, int32_t *size, int32_t *full, int32_t *mem, int32_t buffer_size);
void SubBuffer(float_t *in, int32_t in_size, float_t *out, int32_t out_size, int32_t start);

#ifdef __cplusplus
}
#endif

#endif /* BUFFERS_H */
