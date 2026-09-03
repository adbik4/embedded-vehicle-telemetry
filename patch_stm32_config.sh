#!/bin/bash
set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Core/Inc/stm32u3xx_nucleo_conf.h
sed -i '/#define BSP_BUTTON_USER_IT_PRIORITY  *15U/a #define USE_NUCLEO_144                      1U' \
    "$ROOT/Core/Inc/stm32u3xx_nucleo_conf.h"

# Core/Src/stm32u3xx_it.c
sed -i 's/HAL_EXTI_IRQHandler(&H_EXTI_13);/BSP_PB_IRQHandler(BUTTON_USER);/' \
    "$ROOT/Core/Src/stm32u3xx_it.c"

# Library/App/app_algobuild.c
sed -i 's/BUTTON_KEY/BUTTON_USER/g; s/\bLED2\b/LED_RED/g' \
    "$ROOT/Library/App/app_algobuild.c"

# Library/Target/com.c
sed -i 's/__HAL_DMA_GET_TE_FLAG_INDEX(handle_dma)/DMA_FLAG_DTE/' \
    "$ROOT/Library/Target/com.c"

# cmake/stm32cubemx/CMakeLists.txt
sed -i 's/target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})/target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})/' \
    "$ROOT/cmake/stm32cubemx/CMakeLists.txt"


# Core/Src/main.c
python3 - "$ROOT/Core/Src/main.c" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

old = """  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
"""

if old in text:
    text = text.replace(old, "", 1)
elif "BspCOMInit.BaudRate   = 115200;" in text:
    raise SystemExit("main.c: COM1 initialization block is incomplete")

path.write_text(text)
PY


# Drivers/BSP/Components/lsm6dsv16x/lsm6dsv16x.c
python3 - "$ROOT/Drivers/BSP/Components/lsm6dsv16x/lsm6dsv16x.c" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

anchor = """  if (lsm6dsv16x_ah_qvar_mode_set(&(pObj->Ctx), mode) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }
"""

insert = """  /* Enable accelerometer data-ready interrupt on INT1 pin */
  if (LSM6DSV16X_ACC_Enable_DRDY_On_INT1(pObj) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }

  /* Enable gyroscope data-ready interrupt on INT2 pin */
  if (LSM6DSV16X_GYRO_Enable_DRDY_On_INT2(pObj) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }

"""

if insert in text:
    pass
elif anchor in text:
    text = text.replace(anchor, anchor + insert, 1)
else:
    raise SystemExit("lsm6dsv16x.c: insertion point not found")

path.write_text(text)
PY


# Drivers/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.c
python3 - "$ROOT/Drivers/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.c" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

# Add DMA declaration once
declaration = "extern DMA_HandleTypeDef handle_GPDMA1_Channel0;\n"

text = text.replace(declaration, "")

anchor = """#if defined(__ICCARM__)
#include <LowLevelIOInterface.h>
#endif /* __ICCARM__ */
"""

if anchor not in text:
    raise SystemExit("stm32u3xx_nucleo.c: ICCARM include block not found")

text = text.replace(anchor, anchor + declaration, 1)


# COM_Init / DMA setup
old = """    if (MX_USART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
"""

new = """    if (COM_Init != NULL && MX_USART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (COM == COM1)
    {
      hcom_uart[COM].hdmarx = &handle_GPDMA1_Channel0;
      handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
      handle_GPDMA1_Channel0.Init.Request = GPDMA1_REQUEST_USART1_RX;
      handle_GPDMA1_Channel0.Init.Direction = DMA_PERIPH_TO_MEMORY;
      handle_GPDMA1_Channel0.Init.SrcInc = DMA_SINC_FIXED;
      handle_GPDMA1_Channel0.Init.DestInc = DMA_DINC_INCREMENTED;
      handle_GPDMA1_Channel0.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
      handle_GPDMA1_Channel0.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
      handle_GPDMA1_Channel0.Init.Priority = DMA_HIGH_PRIORITY;
      handle_GPDMA1_Channel0.Init.SrcBurstLength = 1U;
      handle_GPDMA1_Channel0.Init.DestBurstLength = 1U;
      handle_GPDMA1_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
      handle_GPDMA1_Channel0.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
      handle_GPDMA1_Channel0.Init.Mode = DMA_NORMAL;
      handle_GPDMA1_Channel0.Parent = &hcom_uart[COM];

      if (HAL_DMA_Init(&handle_GPDMA1_Channel0) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
"""

if new not in text:
    if old not in text:
        raise SystemExit("stm32u3xx_nucleo.c: USART1 initialization block not found")
    text = text.replace(old, new, 1)

# COM1_MspInit
anchor = """  /* Enable USART clock */
  COM1_CLK_ENABLE();
"""

dma = """  /* Enable DMA clock and configure USART1 RX DMA */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
  handle_GPDMA1_Channel0.Init.Request = GPDMA1_REQUEST_USART1_RX;
  handle_GPDMA1_Channel0.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  handle_GPDMA1_Channel0.Init.Direction = DMA_PERIPH_TO_MEMORY;
  handle_GPDMA1_Channel0.Init.SrcInc = DMA_SINC_FIXED;
  handle_GPDMA1_Channel0.Init.DestInc = DMA_DINC_INCREMENTED;
  handle_GPDMA1_Channel0.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
  handle_GPDMA1_Channel0.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
  handle_GPDMA1_Channel0.Init.Priority = DMA_HIGH_PRIORITY;
  handle_GPDMA1_Channel0.Init.SrcBurstLength = 1U;
  handle_GPDMA1_Channel0.Init.DestBurstLength = 1U;
  handle_GPDMA1_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  handle_GPDMA1_Channel0.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  handle_GPDMA1_Channel0.Init.Mode = DMA_NORMAL;

  HAL_DMA_Init(&handle_GPDMA1_Channel0);

  __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel0);

"""

if dma not in text:
    if anchor not in text:
        raise SystemExit("stm32u3xx_nucleo.c: COM1 clock initialization not found")
    text = text.replace(anchor, anchor + dma, 1)


# COM1_MspDeInit DMA cleanup
anchor = """/**
  * @brief  Initialize USART3 Msp part
  * @param  huart UART handle
  * @retval BSP status
  */
static void COM1_MspDeInit(UART_HandleTypeDef *huart)
"""

cleanup = """  if (huart->hdmarx != NULL)
  {
    (void)HAL_DMA_DeInit(huart->hdmarx);
    huart->hdmarx = NULL;
  }
"""

if cleanup not in text:
    deinit_start = text.find(anchor)

    if deinit_start == -1:
        raise SystemExit("stm32u3xx_nucleo.c: COM1_MspDeInit function not found")

    pos = text.find("  UNUSED(huart);", deinit_start)

    if pos == -1:
        raise SystemExit("stm32u3xx_nucleo.c: UNUSED(huart) in COM1_MspDeInit not found")

    text = text[:pos] + cleanup + text[pos + len("  UNUSED(huart);\n"):]

path.write_text(text)
PY


# Drivers/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.h
python3 - "$ROOT/Drivers/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.h" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

text = text.replace(
"""  LED_BLUE  = LD3,
  LEDn
""",
"""  LED_BLUE  = LD3,
  /* alias */
  LED2 = LD2,
  LEDn
""",
1
)

text = text.replace(
"""  BUTTON_USER = B1,
  BUTTONn
""",
"""  BUTTON_USER = B1,
  /* Alias */
  BUTTON_KEY  = BUTTON_USER,
  BUTTONn
""",
1
)

path.write_text(text)
PY


# Library/App/app_algobuild.c
python3 - "$ROOT/Library/App/app_algobuild.c" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

old = " BSP_COM_Init(COM1);"

new = """  COM_InitTypeDef huart1;
  huart1.BaudRate = 921600;
  huart1.WordLength = UART_WORDLENGTH_8B;
  huart1.StopBits = UART_STOPBITS_1;
  huart1.Parity = UART_PARITY_NONE;
  huart1.HwFlowCtl = UART_HWCONTROL_NONE;
  BSP_COM_Init(COM1, &huart1);"""

if old in text:
    text = text.replace(old, new, 1)
elif "BSP_COM_Init(COM1, &huart1);" not in text:
    raise SystemExit("app_algobuild.c: BSP_COM_Init(COM1) not found")

path.write_text(text)
PY


echo "STM32 patches applied successfully."