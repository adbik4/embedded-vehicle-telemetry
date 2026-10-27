#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Core/Src/main.c
sed -i \
    -e '/MX_Library_Init();/d' \
    -e '/MX_Library_Process();/d' \
    "$ROOT/Core/Src/main.c"

# Core/Inc/stm32u3xx_nucleo_conf.h
CONF_FILE="$ROOT/Core/Inc/stm32u3xx_nucleo_conf.h"

if ! grep -q '^#define USE_NUCLEO_144[[:space:]]' "$CONF_FILE"; then
    sed -i \
        '/#define USE_BSP_COM_FEATURE[[:space:]]*1U/a\
#define USE_NUCLEO_144                      1U' \
        "$CONF_FILE"
fi

# Library/App/app_algobuild.c
sed -i \
    -e 's/BUTTON_KEY/BUTTON_USER/g' \
    -e 's/\bLED2\b/LED_RED/g' \
    -e '/\/\* Initialize selected USART \*\//,+1d' \
    -e '/^void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef \*htim)/,/^}/d' \
    "$ROOT/Library/App/app_algobuild.c"

# Library/Target/com.c
COM_FILE="$ROOT/Library/Target/com.c"

sed -i \
    's/__HAL_DMA_GET_TE_FLAG_INDEX(handle_dma)/DMA_FLAG_DTE/g' \
    "$COM_FILE"

if ! grep -q '^#define COM1 0U$' "$COM_FILE"; then
    sed -i \
        '/^#define UART_MSG_MAX_SIZE TMsg_MaxLen$/a\
#define COM1 0U' \
        "$COM_FILE"
fi

# cmake/stm32cubemx/CMakeLists.txt
CMAKE_FILE="$ROOT/cmake/stm32cubemx/CMakeLists.txt"

sed -i \
    's/target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})/target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})/' \
    "$CMAKE_FILE"

# Replace source paths literally, without regex-escaping errors.
sed -i \
    -e 's|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/app_freertos.c|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/app_freertos.cpp|g' \
    -e 's|${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/custom_mems_control_ex.c|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_mems_control_ex.c|g' \
    -e '/custom_mems_control_ex\.c/a\    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_algobuild.c' \
    "$CMAKE_FILE"

# Core/Src/app_freertos.c
FREERTOS_FILE="$ROOT/Core/Src/app_freertos.cpp"

sed -i -E \
    -e 's/uint32_t SystemTaskBuffer\[ *[0-9]+ *\]/uint32_t SystemTaskBuffer[ SYSTEM_TASK_STACK_SIZE ]/' \
    -e 's/\.priority[[:space:]]*=[[:space:]]*\(osPriority_t\)[[:space:]]*[[:alpha:]_]+[[:space:]]*,/.priority = SYSTEM_TASK_PRIO,/' \
    "$FREERTOS_FILE"

# Library/App/ab_libraries.h
sed -i \
    's/#define MotionFX_6X  0/#define MotionFX_6X  1/' \
    "$ROOT/Library/App/ab_libraries.h"

echo "STM32 patches applied successfully."