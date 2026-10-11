#!/bin/bash
set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Core/Src/main.c
sed -i \
    -e '/MX_Library_Init();/d' \
    -e '/MX_Library_Process();/d' \
  "$ROOT/Core/Src/main.c"

# Core/Inc/stm32u3xx_nucleo_conf.h
sed -i '/#define BSP_BUTTON_USER_IT_PRIORITY  *15U/a #define USE_NUCLEO_144                      1U' \
    "$ROOT/Core/Inc/stm32u3xx_nucleo_conf.h"

# Library/App/app_algobuild.c
sed -i \
    -e 's/BUTTON_KEY/BUTTON_USER/g; s/\bLED2\b/LED_RED/g' \
    -e '/\/\* Initialize selected USART \*\//,+1d' \
    "$ROOT/Library/App/app_algobuild.c"

# Library/Target/com.c
COM_FILE="$ROOT/Library/Target/com.c"
sed -i 's/__HAL_DMA_GET_TE_FLAG_INDEX(handle_dma)/DMA_FLAG_DTE/' \
    "$COM_FILE"

if ! grep -q '^#define COM1 0U$' "$COM_FILE"; then
    sed -i '/^#define UART_MSG_MAX_SIZE TMsg_MaxLen$/a #define COM1 0U' \
    "$COM_FILE"
fi

# cmake/stm32cubemx/CMakeLists.txt
CMAKE_FILE="$ROOT/cmake/stm32cubemx/CMakeLists.txt"

sed -i \
    -e 's/target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})/target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})/' \
    "$CMAKE_FILE"

if ! grep -Fq '${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_algobuild.c' "$CMAKE_FILE"; then
    sed -i \
        -e '\|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls\.c|a\
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_algobuild.c' \
        "$CMAKE_FILE"
fi

echo "STM32 patches applied successfully."