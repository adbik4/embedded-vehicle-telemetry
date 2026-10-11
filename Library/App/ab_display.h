/**
  ******************************************************************************
  * @file    ab_display.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_display.c
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
#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Serial message array indexes enumeration based on DataLogCustomLite application
  */
typedef enum
{
  DEST        = 0,
  SOURCE      = DEST      + 1,
  COMMAND     = SOURCE    + 1,
  TIMESTAMP   = COMMAND   + 1,
  STREAM_DATA = TIMESTAMP + 6,
} MSG_INDEXES;

typedef struct
{
  uint8_t info_type;
  uint8_t info_index;
  uint16_t variable_count;
  uint8_t variable_type;
  uint16_t stream_position;
  const char *config_string;
  uint8_t already_processed;
  void *p_node;
} sDISPLAY_INFO, *psDISPLAY_INFO_t;

typedef struct
{
  uint8_t var_count;
  uint8_t var_type;
  uint8_t conn_size;
  uint8_t conn[1];
} sCONFIG_RECORD_t;

/* Exported constants --------------------------------------------------------*/
#define VAR_TYPE_FLOAT    1
#define VAR_TYPE_INT32    2
#define VAR_TYPE_BIT      3
#define VAR_TYPE_INPUT    0x80

#define INFO_TYPE_ABITS   1
#define INFO_TYPE_AINT32  2
#define INFO_TYPE_AFLOAT  3
#define INFO_TYPE_GRAPH   4
#define INFO_TYPE_LOGIC   5
#define INFO_TYPE_SCATTER 6
#define INFO_TYPE_FUSION  7
#define INFO_TYPE_INT32   8
#define INFO_TYPE_FLOAT   9
#define INFO_TYPE_BAR     10
#define INFO_TYPE_HISTO   11
#define INFO_TYPE_PLOT3D  12
#define INFO_TYPE_FFT     13
#define INFO_TYPE_ANGLE   14

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Display_Update(void *source, sDISPLAY_INFO *info);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */
