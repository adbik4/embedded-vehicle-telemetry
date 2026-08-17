/**
  ******************************************************************************
  * @file    bsp_ip_conf.h
  * @author  MEMS Software Solutions Team
  * @brief   BSP IP configuration file
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
#ifndef __BSP_IP_CONF_H__
#define __BSP_IP_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Exported functions --------------------------------------------------------*/
void MX_TIM4_Init(void);
void MX_TIM1_Init(void);

/* Exported defines ----------------------------------------------------------*/
extern TIM_HandleTypeDef htim4;
#define BSP_IP_TIM_HANDLE_AB   htim4
#define BSP_IP_TIM_INIT_AB     MX_TIM4_Init

extern TIM_HandleTypeDef htim1;
#define BSP_IP_TIM_HANDLE_AL   htim1
#define BSP_IP_TIM_INIT_AL     MX_TIM1_Init

/* Exported variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;

#ifdef __cplusplus
}
#endif

#endif /* __BSP_IP_CONF_H__ */
