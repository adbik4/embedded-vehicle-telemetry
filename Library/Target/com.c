/**
  ******************************************************************************
  * @file    com.c
  * @author  MEMS Software Solutions Team
  * @brief   This file provides communication functions
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
#include "com.h"
#include "ab_sensor_hub.h"
#include "bsp_ip_conf.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Communication
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UART_MSG_MAX_SIZE TMsg_MaxLen

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
volatile uint8_t UART_RxBuffer[UART_RX_BUFFER_SIZE];
volatile uint8_t UART_TxBuffer[TMsg_MaxLen * 2];

extern UART_HandleTypeDef hcom_uart[];

TUart_Engine_t Uart_Engine;

extern TMsg MsgDat;

/* Private function prototypes -----------------------------------------------*/
uint32_t Get_DMA_Flag_Status(DMA_HandleTypeDef *handle_dma);
uint32_t Get_DMA_Counter(DMA_HandleTypeDef *handle_dma);

/* Exported functions --------------------------------------------------------*/
void StartReceiveMsg(void)
{
  hcom_uart[COM1].pRxBuffPtr = (uint8_t *)UART_RxBuffer;
  hcom_uart[COM1].RxXferSize = UART_RX_BUFFER_SIZE;
  hcom_uart[COM1].ErrorCode = HAL_UART_ERROR_NONE;

  /* Enable the DMA transfer for the receiver request by setting the DMAR bit
  in the UART CR3 register */
  HAL_UART_Receive_DMA(&hcom_uart[COM1], (uint8_t *)UART_RxBuffer, UART_RX_BUFFER_SIZE);
}

/**
  * @brief  Check if a message is received via UART
  * @param  Msg the pointer to the message to be received
  * @retval 1 if a complete message is found, 0 otherwise
  */
int32_t ReceivedMSG(TMsg *Msg)
{
  uint16_t i, j, k, j2;
  uint16_t DmaCounter, length;
  uint8_t Data;
  uint16_t source = 0;
  uint8_t inc = 0;

  if (Get_DMA_Flag_Status(hcom_uart[COM1].hdmarx) == RESET)
  {
    DmaCounter = UART_RX_BUFFER_SIZE - Get_DMA_Counter(hcom_uart[COM1].hdmarx);

    if (DmaCounter >= Uart_Engine.StartOfMsg)
    {
      length = DmaCounter - Uart_Engine.StartOfMsg;
    }

    else
    {
      length = UART_RX_BUFFER_SIZE + DmaCounter - Uart_Engine.StartOfMsg;
    }

    j = Uart_Engine.StartOfMsg;

    for (k = 0; k < length; k++)
    {
      Data = UART_RxBuffer[j];
      j++;
      if (j >= UART_RX_BUFFER_SIZE)
      {
        j = 0;
      }

      if (Data == TMsg_EOF)
      {
        j = Uart_Engine.StartOfMsg;
        for (i = 0; i < k; i += inc)
        {
          uint8_t Source0;
          uint8_t Source1;
          uint8_t *Dest;

          j2 = (j + 1) % UART_RX_BUFFER_SIZE;

          Source0 = UART_RxBuffer[j];
          Source1 = UART_RxBuffer[j2];
          Dest    = &Msg->Data[source];

          inc = ReverseByteStuffCopyByte2(Source0, Source1, Dest);
          if (inc == 0)
          {
            Uart_Engine.StartOfMsg = j2;
            return 0;
          }

          j = (j + inc) % UART_RX_BUFFER_SIZE;
          source++;
        }

        Msg->Len = source;
        j = (j + 1) % UART_RX_BUFFER_SIZE; /* skip TMsg_EOF */
        Uart_Engine.StartOfMsg = j;

        if (CHK_CheckAndRemove(Msg)) /* check message integrity */
        {
          return 1;
        }
      }
    }

    if (length > UART_MSG_MAX_SIZE)
    {
      Uart_Engine.StartOfMsg = DmaCounter;
    }
  }

  return 0;
}

/**
  * @brief  Send a message via UART
  * @param  Msg the pointer to the message to be sent
  * @retval None
  */
void SendMsg(TMsg *Msg)
{
  uint16_t CountOut;
  CHK_ComputeAndAdd(Msg);

  CountOut = ByteStuffCopy((uint8_t *)UART_TxBuffer, Msg);
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)UART_TxBuffer, CountOut, 5000);
}

/**
  * @brief  Send FFT data via UART
  * @param  data pointer to FFT data
  * @param  size of data (number of floats)
  * @retval None
  */
void SendFFT(float_t *data, uint32_t size, uint8_t config_index)
{
  uint16_t CountOut;
  uint8_t header[8];
  uint8_t CHK;
  uint8_t buffer[512];
  uint32_t copy_size;
  uint32_t index;
  float_t *sf = (float_t *)(&header[4]);

  header[0] = 1; //DataStreamingDest;
  header[1] = 50; //DEV_ADDR;
  header[2] = 0x7d; //CMD_SF_DATA;
  header[3] = config_index;
  *sf = sensor_hub_data.meas_data_rate_Hz;
  CHK = CHK_Compute(header, 8, 0);
  CHK = CHK_Compute((uint8_t *)data, size * 4, CHK);
  HAL_UART_Transmit(&hcom_uart[COM1], header, 8, 5000);

  index = 0;
  while (size > 0)
  {
    copy_size = 64;
    if (copy_size > size)
    {
      copy_size = size; //number of floats
    }
    CountOut = ByteStuffCopyBuffer(buffer, (uint8_t *)(&data[index]), copy_size * 4);
    HAL_UART_Transmit(&hcom_uart[COM1], buffer, CountOut, 5000);
    index += copy_size;
    size -= copy_size;
  }

  CountOut = ByteStuffCopyBuffer(header, &CHK, 1);
  header[CountOut] = TMsg_EOF;
  CountOut++;
  HAL_UART_Transmit(&hcom_uart[COM1], header, CountOut, 5000);
}

/**
  * @brief  Build an array from the source data (LSB first)
  * @param  Dest destination
  * @param  Source source
  * @param  Len number of bytes
  * @retval None
  */
void SerializeToMsg(uint8_t Dest, void *Source, uint32_t Len)
{
  memcpy(&MsgDat.Data[Dest], Source, Len);
}

/**
  * @brief  Get the DMA Stream pending flags
  * @param  handle_dma DMA handle
  * @retval The state of FLAG (SET or RESET)
  */
uint32_t Get_DMA_Flag_Status(DMA_HandleTypeDef *handle_dma)
{
  return (__HAL_DMA_GET_FLAG(handle_dma, DMA_FLAG_DTE));
}

/**
  * @brief  Returns the number of remaining data units in the current DMAy Streamx transfer
  * @param  handle_dma DMA handle
  * @retval The number of remaining data units in the current DMA Stream transfer
  */
uint32_t Get_DMA_Counter(DMA_HandleTypeDef *handle_dma)
{
  return (__HAL_DMA_GET_COUNTER(handle_dma));
}

/**
  * @}
  */

/**
  * @}
  */
