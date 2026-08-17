/**
  ******************************************************************************
  * @file    serial_protocol.c
  * @author  MEMS Software Solutions Team
  * @brief   This file implements utilities for the serial protocol
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

/* Includes ------------------------------------------------------------------*/
#include "serial_protocol.h"
#include "com.h"
#include <string.h>

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Serial_Protocol
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Byte stuffing process for one byte
  * @param  Dest destination
  * @param  Source source
  * @retval Total number of bytes processed
  */
int32_t ByteStuffCopyByte(uint8_t *Dest, uint8_t Source)
{
  int32_t ret = 2;

  switch (Source)
  {
    case TMsg_EOF:
      Dest[0] = TMsg_BS;
      Dest[1] = TMsg_BS_EOF;
      break;

    case TMsg_BS:
      Dest[0] = TMsg_BS;
      Dest[1] = TMsg_BS;
      break;

    default:
      Dest[0] = Source;
      ret = 1;
      break;
  }

  return ret;
}

/**
  * @brief  Byte stuffing process for a Msg
  * @param  Dest destination
  * @param  Source source
  * @retval Total number of bytes processed
  */
int32_t ByteStuffCopy(uint8_t *Dest, TMsg *Source)
{
  uint32_t i;
  int32_t count = 0;

  for (i = 0; i < Source->Len; i++)
  {
    count += ByteStuffCopyByte(&Dest[count], Source->Data[i]);
  }

  Dest[count] = TMsg_EOF;
  count++;
  return count;
}

/**
  * @brief  Byte stuffing process for a buffer
  * @param  Dest destination
  * @param  data pointer to source data
  * @param  size data size
  * @retval Total number of bytes processed
  */
int32_t ByteStuffCopyBuffer(uint8_t *Dest, uint8_t *data, int32_t size)
{
  int32_t i;
  int32_t count = 0;

  for (i = 0; i < size; i++)
  {
    count += ByteStuffCopyByte(&Dest[count], data[i]);
  }
  return count;
}

/**
  * @brief  Reverse Byte stuffing process for one byte
  * @param  Source source
  * @param  Dest destination
  * @retval Number of input bytes processed (1 or 2) or 0 for invalid sequence
  */
int32_t ReverseByteStuffCopyByte(uint8_t *Source, uint8_t *Dest)
{
  if (Source[0] == (uint8_t)TMsg_BS)
  {
    if (Source[1] == (uint8_t)TMsg_BS)
    {
      *Dest = TMsg_BS;
      return 2;
    }

    if (Source[1] == (uint8_t)TMsg_BS_EOF)
    {
      *Dest = TMsg_EOF;
      return 2;
    }

    return 0; /* Invalid sequence */
  }
  else
  {
    *Dest = Source[0];
    return 1;
  }
}

/**
  * @brief  Reverse Byte stuffing process for two input data
  * @param  Source0 input data
  * @param  Source1 input data
  * @param  Dest the destination data
  * @retval Number of input bytes processed (1 or 2) or 0 for invalid sequence
  */
int32_t ReverseByteStuffCopyByte2(uint8_t Source0, uint8_t Source1, uint8_t *Dest)
{
  if (Source0 == (uint8_t)TMsg_BS)
  {
    if (Source1 == (uint8_t)TMsg_BS)
    {
      *Dest = TMsg_BS;
      return 2;
    }

    if (Source1 == (uint8_t)TMsg_BS_EOF)
    {
      *Dest = TMsg_EOF;
      return 2;
    }

    return 0; /* Invalid sequence */
  }
  else
  {
    *Dest = Source0;
    return 1;
  }
}

/**
  * @brief  Reverse Byte stuffing process for a Msg
  * @param  Dest destination
  * @param  Source source
  * @retval 1 if the operation succeeds, 0 if an error occurs
  */
int32_t ReverseByteStuffCopy(TMsg *Dest, uint8_t *Source)
{
  uint32_t count = 0;
  int32_t state = 0;
  uint8_t *source_local = Source;

  while ((*source_local) != (uint8_t)TMsg_EOF)
  {
    if (state == 0)
    {
      if ((*source_local) == (uint8_t)TMsg_BS)
      {
        state = 1;
      }
      else
      {
        Dest->Data[count] = *source_local;
        count++;
      }
    }
    else
    {
      if ((*source_local) == (uint8_t)TMsg_BS)
      {
        Dest->Data[count] = TMsg_BS;
        count++;
      }
      else
      {
        if ((*source_local) == (uint8_t)TMsg_BS_EOF)
        {
          Dest->Data[count] = TMsg_EOF;
          count++;
        }
        else
        {
          return 0; /* Invalid sequence */
        }
      }

      state = 0;
    }

    source_local++;
  }

  if (state != 0)
  {
    return 0;
  }

  Dest->Len = count;
  return 1;
}

/**
  * @brief  Compute and add checksum
  * @param  Msg pointer to the message
  * @retval None
  */
void CHK_ComputeAndAdd(TMsg *Msg)
{
  uint8_t chk = 0;
  uint32_t i;

  for (i = 0; i < Msg->Len; i++)
  {
    chk -= Msg->Data[i];
  }

  Msg->Data[i] = chk;
  Msg->Len++;
}

/**
  * @brief  Compute checksum
  * @param  data - pointer to data
  *         size - length of data
  * @retval Checksum
  */
uint8_t CHK_Compute(uint8_t *data, uint32_t size, uint8_t CHK_ini)
{
  uint8_t CHK = CHK_ini;
  uint32_t i;

  for (i = 0; i < size; i++)
  {
    CHK -= data[i];
  }
  return CHK;
}

/**
  * @brief  Compute and remove checksum
  * @param  Msg pointer to the message
  * @retval A number different from 0 if the operation succeeds, 0 if an error occurs
  */
int32_t CHK_CheckAndRemove(TMsg *Msg)
{
  uint8_t chk = 0;
  uint32_t i;

  for (i = 0; i < Msg->Len; i++)
  {
    chk += Msg->Data[i];
  }

  Msg->Len--;
  return (int32_t)(chk == 0U);
}

/**
  * @brief  Build an array from the source data (LSB first)
  * @param  Dest destination
  * @param  Source source
  * @param  Len number of bytes
  * @retval None
  */
void Serialize(uint8_t Dest, void *Source, uint32_t Len)
{
  SerializeToMsg(Dest, Source, Len);
}

/**
  * @brief  Unbuild a Number from an array (LSB first)
  * @param  Source source
  * @param  Len number of bytes
  * @retval Rebuild uint32_t variable
  */
uint32_t Deserialize(uint8_t *Source, uint32_t Len)
{
  uint32_t app;
  uint32_t len_local = Len;

  --len_local;
  app = (uint32_t)Source[len_local];
  while (len_local > 0U)
  {
    app <<= 8;
    --len_local;
    app += (uint32_t)Source[len_local];
  }

  return app;
}

/**
  * @brief  Build an array from the uint32_t (LSB first)
  * @param  Dest destination
  * @param  Source source
  * @param  Len number of bytes
  * @retval None
  */
void Serialize_s32(uint8_t *Dest, int32_t Source, uint32_t Len)
{
  uint32_t i;
  uint32_t source_uint32 = (uint32_t)Source;

  for (i = 0; i < Len; i++)
  {
    Dest[i] = (uint8_t)(source_uint32 & 0xFFU);
    source_uint32 >>= 8;
  }
}

/**
  * @brief  Unbuild a Number from an array (LSB first)
  * @param  Source source
  * @param  Len number of bytes
  * @retval Rebuild signed int32_t variable
  */
int32_t Deserialize_s32(uint8_t *Source, uint32_t Len)
{
  uint32_t app;
  uint32_t len_local = Len;

  --len_local;
  app = (uint32_t)Source[len_local];
  while (len_local > 0U)
  {
    app <<= 8;
    --len_local;
    app += (uint32_t)Source[len_local];
  }

  return (int32_t)app;
}

/**
  * @}
  */

/**
  * @}
  */
