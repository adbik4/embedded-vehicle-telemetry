# Patch STM32 project configuration files to:
# - add the missing Nucleo board define
# - update BSP button/LED compatibility
# - fix the EXTI handler
# - remove the COM1 initialisation from main.c
# - add LSM6DSV16X DRDY interrupt configuration
# - configure the Nucleo COM1 DMA using USART1 RX only
# - update the app AlgoBuild COM initialisation
# - fix the DMA flag access in com.c
# - ensure the CMake link line uses PRIVATE for MX_LINK_LIBS

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

function Update-File {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][scriptblock]$Transform
    )

    $path = Join-Path $root $RelativePath

    if (-not (Test-Path $path)) {
        throw "Missing file: $path"
    }

    $original = Get-Content -Path $path -Raw
    $updated = & $Transform $original

    if ($updated -ne $original) {
        Set-Content -Path $path -Value $updated -NoNewline
        Write-Host "Updated: $RelativePath"
    }
    else {
        Write-Host "Already up to date: $RelativePath"
    }
}

# ---------------------------------------------------------------------------
# Core/Inc/stm32u3xx_nucleo_conf.h
# ---------------------------------------------------------------------------

Update-File 'Core/Inc/stm32u3xx_nucleo_conf.h' {
    param($content)

    if ($content -notmatch '#define USE_NUCLEO_144') {
        $content = [regex]::Replace(
            $content,
            '(?m)^#define BSP_BUTTON_USER_IT_PRIORITY\s+15U\s*$',
            '$0' + [Environment]::NewLine + '#define USE_NUCLEO_144                      1U',
            1
        )
    }

    return $content
}

# ---------------------------------------------------------------------------
# Core/Src/stm32u3xx_it.c
# ---------------------------------------------------------------------------

Update-File 'Core/Src/stm32u3xx_it.c' {
    param($content)

    $content = $content -replace `
        'HAL_EXTI_IRQHandler\(&H_EXTI_13\);', `
        'BSP_PB_IRQHandler(BUTTON_USER);'

    return $content
}

# ---------------------------------------------------------------------------
# Core/Src/main.c
# Remove the COM1 initialisation block.
# ---------------------------------------------------------------------------

Update-File 'Core/Src/main.c' {
    param($content)

    $snippet = @'
  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
'@

    if ($content.Contains($snippet)) {
        $content = $content.Replace($snippet, '')
    }
    elseif ($content -match 'BSP_COM_Init\(COM1,\s*&BspCOMInit\)') {
        throw "Found BSP_COM_Init(COM1, &BspCOMInit) in main.c, but the expected COM1 initialisation block does not exactly match."
    }

    return $content
}

# ---------------------------------------------------------------------------
# Library/App/app_algobuild.c
#
# - Update button/LED names.
# - Replace BSP_COM_Init(COM1) with explicit 921600 baud configuration.
# ---------------------------------------------------------------------------

Update-File 'Library/App/app_algobuild.c' {
    param($content)

    $content = $content -replace 'BUTTON_KEY', 'BUTTON_USER'
    $content = $content -replace '\bLED2\b', 'LED_RED'

    $oldComInit = 'BSP_COM_Init(COM1);'

    $newComInit = @'
  COM_InitTypeDef huart1;
  huart1.BaudRate = 921600;
  huart1.WordLength = UART_WORDLENGTH_8B;
  huart1.StopBits = UART_STOPBITS_1;
  huart1.Parity = UART_PARITY_NONE;
  huart1.HwFlowCtl = UART_HWCONTROL_NONE;
  BSP_COM_Init(COM1, &huart1);
'@

    if ($content.Contains($oldComInit)) {
        $content = $content.Replace($oldComInit, $newComInit)
    }

    return $content
}

# ---------------------------------------------------------------------------
# Library/Target/lsm6dsv16x.c
#
# Add accelerometer DRDY -> INT1
# Add gyroscope DRDY       -> INT2
# ---------------------------------------------------------------------------

Update-File 'Drivers\BSP\Components\lsm6dsv16x\lsm6dsv16x.c' {
    param($content)

    $anchor = @'
  if (lsm6dsv16x_ah_qvar_mode_set(&(pObj->Ctx), mode) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }
'@

    $insertion = @'

  /* Enable accelerometer data-ready interrupt on INT1 pin */
  if (LSM6DSV16X_ACC_Enable_DRDY_On_INT1(pObj) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }

  /* Enable gyroscope data-ready interrupt on INT2 pin */
  if (LSM6DSV16X_GYRO_Enable_DRDY_On_INT2(pObj) != LSM6DSV16X_OK)
  {
    return LSM6DSV16X_ERROR;
  }
'@

    if ($content -notmatch 'LSM6DSV16X_ACC_Enable_DRDY_On_INT1\(pObj\)') {
        if (-not $content.Contains($anchor)) {
            throw "Could not find LSM6DSV16X AH/QVAR mode configuration in lsm6dsv16x.c."
        }

        $content = $content.Replace($anchor, $anchor + $insertion)
    }

    return $content
}

# ---------------------------------------------------------------------------
# Library/Target/stm32u3xx_nucleo.c
# ---------------------------------------------------------------------------

Update-File 'Drivers\BSP\STM32U3xx_Nucleo\stm32u3xx_nucleo.c' {
    param($content)

    # -----------------------------------------------------------------------
    # Add DMA handle declaration
    # -----------------------------------------------------------------------

    $includeAnchor = @'
#endif /* __ICCARM__ */
'@

    $dmaDeclaration = @'

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
'@

    if ($content -notmatch '\bhandle_GPDMA1_Channel0\b') {
        if (-not $content.Contains($includeAnchor)) {
            throw "Could not find ICCARM include block in stm32u3xx_nucleo.c."
        }

        $content = $content.Replace(
            $includeAnchor,
            $includeAnchor + $dmaDeclaration
        )
    }

    # -----------------------------------------------------------------------
    # Replace COM1 USART initialisation logic.
    #
    # DMA is configured in COM1_MspInit(), so there is no second DMA
    # configuration here.
    # -----------------------------------------------------------------------

    $oldComBlock = @'
    if (MX_USART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
'@

    $newComBlock = @'
    if (COM_Init != NULL && MX_USART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
'@

    if ($content.Contains($oldComBlock)) {
        $content = $content.Replace($oldComBlock, $newComBlock)
    }

    # -----------------------------------------------------------------------
    # COM1_MspInit:
    #
    # Add (void)huart;
    # Configure GPDMA1 Channel 0 for USART1 RX.
    # -----------------------------------------------------------------------

    $mspInitAnchor = @'
static void COM1_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;
'@

    $mspInitWithUnused = @'
static void COM1_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;

  (void)huart;
'@

    if (
        $content.Contains($mspInitAnchor) -and
        $content -notmatch `
        'static void COM1_MspInit\(UART_HandleTypeDef \*huart\)[\s\S]{0,300}\(void\)huart;'
    ) {
        $content = $content.Replace(
            $mspInitAnchor,
            $mspInitWithUnused
        )
    }

    # -----------------------------------------------------------------------
    # Add DMA configuration below COM1_CLK_ENABLE().
    # -----------------------------------------------------------------------

    $com1ClockAnchor = @'
  /* Enable USART clock */
  COM1_CLK_ENABLE();
'@

    $dmaMspCode = @'

  /* Enable DMA clock and configure USART1 RX DMA */
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

  if (HAL_DMA_Init(&handle_GPDMA1_Channel0) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel0);
'@

    if (
        $content.Contains($com1ClockAnchor) -and
        $content -notmatch 'Enable DMA clock and configure USART1 RX DMA'
    ) {
        $content = $content.Replace(
            $com1ClockAnchor,
            $com1ClockAnchor + $dmaMspCode
        )
    }

    # -----------------------------------------------------------------------
    # COM1_MspDeInit:
    #
    # Deinitialise the DMA before deinitialising USART.
    # -----------------------------------------------------------------------

    $deInitAnchor = @'
static void COM1_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef          gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
'@

    $deInitWithDma = @'
static void COM1_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef          gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  if (huart->hdmarx != NULL)
  {
    (void)HAL_DMA_DeInit(huart->hdmarx);
    huart->hdmarx = NULL;
  }
'@

    if (
        $content.Contains($deInitAnchor) -and
        $content -notmatch 'HAL_DMA_DeInit\(huart->hdmarx\)'
    ) {
        $content = $content.Replace(
            $deInitAnchor,
            $deInitWithDma
        )
    }

    return $content
}

# ---------------------------------------------------------------------------
# Library/Target/stm32u3xx_nucleo.h
#
# Add LED2 compatibility alias.
# Add BUTTON_KEY compatibility alias.
# ---------------------------------------------------------------------------

Update-File 'Drivers\BSP\STM32U3xx_Nucleo\stm32u3xx_nucleo.h' {
    param($content)

    $oldLedEnum = @'
typedef enum
{
  LD1 = 0U,
  LED_GREEN = LD1,
  LD2 = 1U,
  LED_RED   = LD2,
  LD3 = 2U,
  LED_BLUE  = LD3,
  LEDn
} Led_TypeDef;
'@

    $newLedEnum = @'
typedef enum
{
  LD1 = 0U,
  LED_GREEN = LD1,
  LD2 = 1U,
  LED_RED   = LD2,
  LD3 = 2U,
  LED_BLUE  = LD3,
  /* alias */
  LED2 = LD2,
  LEDn
} Led_TypeDef;
'@

    if ($content.Contains($oldLedEnum)) {
        $content = $content.Replace($oldLedEnum, $newLedEnum)
    }

    $oldButtonEnum = @'
typedef enum
{
  B1 = 0U,
  BUTTON_USER = B1,
  BUTTONn
} Button_TypeDef;
'@

    $newButtonEnum = @'
typedef enum
{
  B1 = 0U,
  BUTTON_USER = B1,
  /* Alias */
  BUTTON_KEY  = BUTTON_USER,
  BUTTONn
} Button_TypeDef;
'@

    if ($content.Contains($oldButtonEnum)) {
        $content = $content.Replace($oldButtonEnum, $newButtonEnum)
    }

    return $content
}

# ---------------------------------------------------------------------------
# Library/Target/com.c
# ---------------------------------------------------------------------------

Update-File 'Library/Target/com.c' {
    param($content)

    $oldLine = '  return (__HAL_DMA_GET_FLAG(handle_dma, __HAL_DMA_GET_TE_FLAG_INDEX(handle_dma)));'
    $newLine = 'return (__HAL_DMA_GET_FLAG(handle_dma, DMA_FLAG_DTE));'

    if ($content.Contains($oldLine)) {
        $content = $content.Replace($oldLine, $newLine)
    }
    elseif ($content -match '__HAL_DMA_GET_TE_FLAG_INDEX\(handle_dma\)') {
        throw "Found __HAL_DMA_GET_TE_FLAG_INDEX(handle_dma) in com.c, but the expected return statement does not exactly match."
    }

    return $content
}

# ---------------------------------------------------------------------------
# cmake/stm32cubemx/CMakeLists.txt
# ---------------------------------------------------------------------------

Update-File 'cmake/stm32cubemx/CMakeLists.txt' {
    param($content)

    if ($content -match 'target_link_libraries\(\$\{CMAKE_PROJECT_NAME\}\s+PRIVATE\s+\$\{MX_LINK_LIBS\}\)') {
        return $content
    }

    return $content -replace `
        'target_link_libraries\(\$\{CMAKE_PROJECT_NAME\}\s+\$\{MX_LINK_LIBS\}\)', `
        'target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})'
}

Write-Host ''
Write-Host 'Applied STM32 project compatibility fixes.'