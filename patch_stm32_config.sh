
#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Helper: append a line after a matching pattern, only if absent.
insert_after_once() {
    local file="$1"
    local pattern="$2"
    local line="$3"

    if ! grep -Fq "$line" "$file"; then
        sed -i "/$pattern/a\\
$line" "$file"
    fi
}

# Core/Src/main.cpp
MAIN_FILE="$ROOT/Core/Src/main.cpp"

sed -i \
    -e '/MX_Library_Init();/d' \
    -e '/MX_Library_Process();/d' \
    "$MAIN_FILE"

if ! grep -q 'STM32_PATCH_VCP_RX' "$MAIN_FILE"; then
    sed -i \
        '/BSP_COM_Init(COM1, &BspCOMInit)/,/^  }/{
            /^  }/a\
\
  /* STM32_PATCH_VCP_RX */\
  /* Enable interrupts for the VIRTUAL COM port */\
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);\
  HAL_NVIC_EnableIRQ(USART1_IRQn);\
\
  /* Start listening for UART messages */\
  HAL_UARTEx_ReceiveToIdle_IT(&hcom_uart[COM1], (uint8_t*)&rxbuf, 16);
        }' \
        "$MAIN_FILE"
fi

# Core/Inc/stm32u3xx_nucleo_conf.h
CONF_FILE="$ROOT/Core/Inc/stm32u3xx_nucleo_conf.h"

if ! grep -Eq '^#define USE_NUCLEO_144([[:space:]]|$)' "$CONF_FILE"; then
    sed -i \
        '/#define USE_BSP_COM_FEATURE[[:space:]]*1U/a\
#define USE_NUCLEO_144                      1U' \
        "$CONF_FILE"
fi

# Library/App/app_algobuild.c
ALGOBUILD_FILE="$ROOT/Library/App/app_algobuild.c"

sed -i \
    -e 's/BUTTON_KEY/BUTTON_USER/g' \
    -e 's/\<LED2\>/LED_RED/g' \
    -e '/\/\* Initialize selected USART \*\//,+1d' \
    -e '/^void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef \*htim)/,/^}/d' \
    "$ALGOBUILD_FILE"

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
    -e 's/target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})/target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})/' \
    "$CMAKE_FILE"

# Replace source paths literally and only when the extension is .c.
sed -i \
    -e 's|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/app_freertos\.c$|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/app_freertos.cpp|' \
    -e 's|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main\.c$|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.cpp|' \
    -e 's|${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/custom_mems_control_ex\.c$|${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_mems_control_ex.c|' \
    "$CMAKE_FILE"

# Add custom_algobuild.c only once.
insert_after_once \
    "$CMAKE_FILE" \
    'custom_mems_control_ex\.c' \
    '    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/custom_algobuild.c'

# Core/Src/app_freertos.cpp
FREERTOS_FILE="$ROOT/Core/Src/app_freertos.cpp"

sed -i -E \
    -e 's/uint32_t SystemTaskBuffer\[ *[0-9]+ *\]/uint32_t SystemTaskBuffer[ SYSTEM_TASK_STACK_SIZE ]/' \
    -e 's/\.priority[[:space:]]*=[[:space:]]*\(osPriority_t\)[[:space:]]*[[:alpha:]_]+[[:space:]]*,/.priority = SYSTEM_TASK_PRIO,/' \
    "$FREERTOS_FILE"

# Library/App/ab_libraries.h
LIBRARIES_FILE="$ROOT/Library/App/ab_libraries.h"

sed -i \
    -e 's/#define MotionFX_6X  0/#define MotionFX_6X  1/' \
    -e 's/#define MotionFX_9X  0/#define MotionFX_9X  1/' \
    "$LIBRARIES_FILE"

if ! grep -Eq '^#define MotionFX[[:space:]]+1([[:space:]]|$)' "$LIBRARIES_FILE"; then
    sed -i \
        '/#define MotionFX_9X/a #define MotionFX     1' \
        "$LIBRARIES_FILE"
fi

# Remove unnecessary files.
rm -f \
    "$ROOT/Core/Src/main.c" \
    "$ROOT/Core/Src/app_freertos.c"

echo "STM32 patches applied successfully."
