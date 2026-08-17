/**
  *******************************************************************************
  * @file    com.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for com.c
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
#ifndef COM_H
#define COM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "serial_protocol.h"
#include <math.h>

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Serial message engine structure definition
  */
typedef struct
{
  uint8_t *pDMA_Buffer;
  uint16_t StartOfMsg;
} TUart_Engine_t;

/* Exported defines ----------------------------------------------------------*/
#define UART_RX_BUFFER_SIZE (2*TMsg_MaxLen)

/* Exported variables --------------------------------------------------------*/
extern volatile uint8_t UART_RxBuffer[];
extern volatile uint32_t Usart_BaudRate;
extern TUart_Engine_t UartEngine;

/* Exported macro ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void StartReceiveMsg(void);
int32_t  ReceivedMSG(TMsg *Msg);
void SendMsg(TMsg *Msg);
void SendFFT(float_t *data, uint32_t size, uint8_t config_index);
void SerializeToMsg(uint8_t Dest, void *Source, uint32_t Len);

#ifdef __cplusplus
}
#endif

#endif /* COM_H */
