/**
  ******************************************************************************
  * @file    ab_fsm_mlc.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_fsm_mlc.c
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
#ifndef AB_FSM_MLC_H
#define AB_FSM_MLC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

const ucf_line_t fsm_mlc_config[] =
{
  {.address = 0x01, .data = 0x80,},
  {.address = 0x01, .data = 0x00,},
};

#ifdef __cplusplus
}
#endif

#endif /* AB_FSM_MLC_H */
