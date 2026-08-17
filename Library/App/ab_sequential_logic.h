/**
  *******************************************************************************
  * @file    ab_sequential_logic.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_sequential_logic.c
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
#ifndef SEQUENTIAL_LOGIC_H
#define SEQUENTIAL_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void RSFlipFlop(const int32_t *set, const int32_t *reset, int32_t *out, int32_t *outn, int32_t *mem);

#ifdef __cplusplus
}
#endif

#endif /* SEQUENTIAL_LOGIC_H */
