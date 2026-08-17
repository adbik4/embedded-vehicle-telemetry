cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_HAL_DRIVER 
	STM32U3C5xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../X-CUBE-MEMS1/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32U3xx_Nucleo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32U3xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lis2mdl
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionFX_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lsm6dsv16x
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionAC_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionAW_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionEC_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionGC_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionID_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionMC_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionPM_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionPW_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionTL_Library/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/ARM/DSP/Inc
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/com.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/custom_mems_control.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/custom_mems_control_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/Target/app_mems_int_pin_a_interface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/app_algobuild.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_buffers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_fft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_sensor_hub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_sequential_logic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_signal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/ab_user_input.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/algo_builder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/demo_serial.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/sensor_commands.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/App/serial_protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../X-CUBE-MEMS1/Target/custom_motion_sensors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../X-CUBE-MEMS1/Target/custom_motion_sensors_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u3xx_nucleo_bus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u3xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u3xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32u3c5xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32u3xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_gtzc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_icache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_usart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lis2mdl/lis2mdl_reg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lis2mdl/lis2mdl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lsm6dsv16x/lsm6dsv16x_reg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lsm6dsv16x/lsm6dsv16x.c
)

# Drivers Midllewares



# Link directories setup
set(MX_LINK_DIRS

    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionFX_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionAC_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionAW_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionEC_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionFX_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionGC_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionID_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionMC_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionPM_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionPW_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_MotionTL_Library/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/ARM/DSP/Lib
)
# Project static libraries
set(MX_LINK_LIBS 
 "-Wl,--start-group" 
	:MotionFX_CM33F_wc32_ot_hard.a 
	:MotionAC_CM33F_wc32_ot.a 
	:MotionAW_CM33F_wc32_ot.a 
	:MotionEC_CM33F_wc32_ot.a 
	:MotionFX_CM33F_wc32_ot.a 
	:MotionGC_CM33F_wc32_ot.a 
	:MotionID_CM33F_wc32_ot.a 
	:MotionMC_CM33F_wc32_ot.a 
	:MotionPM_CM33F_wc32_ot.a 
	:MotionPW_CM33F_wc32_ot.a 
	:MotionTL_CM33F_wc32_ot.a 
	:libarm_ARMv8MMLlfsp_math.a
 "-Wl,--end-group"
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
