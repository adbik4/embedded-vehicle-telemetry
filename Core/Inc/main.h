/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
// E-paper GPIO
#define EINK_RST_Pin GPIO_PIN_6
#define EINK_RST_GPIO_Port GPIOA
#define EINK_DC_Pin GPIO_PIN_2
#define EINK_DC_GPIO_Port GPIOA
#define EINK_BUSY_Pin GPIO_PIN_3
#define EINK_BUSY_GPIO_Port GPIOA
#define EINK_SPI_CS_Pin GPIO_PIN_4
#define EINK_SPI_CS_GPIO_Port GPIOA
#define EINK_PWR_Pin GPIO_PIN_6
#define EINK_PWR_GPIO_Port GPIOA
#define EINK_DIN_Pin GPIO_PIN_7
#define EINK_DIN_GPIO_Port GPIOA
#define EINK_SCK_Pin GPIO_PIN_5
#define EINK_SCK_GPIO_Port GPIOA

/// L76X GPIO
#define L76X_RX_Pin GPIO_PIN_12
#define L76X_RX_GPIO_Port GPIOB
#define L76X_TX_Pin GPIO_PIN_13
#define L76X_TX_GPIO_Port GPIOB
#define L76X_RST_Pin GPIO_PIN_1
#define L76X_RST_GPIO_Port GPIOB
#define L76X_PPS_Pin GPIO_PIN_2
#define L76X_PPS_GPIO_Port GPIOB

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
