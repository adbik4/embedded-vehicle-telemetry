cmake_minimum_required(VERSION 3.22)

add_library(l76x_lib STATIC
    Config/L76X_DEV_Config.c
    Device/L76X.c
)

target_link_libraries(l76x_lib PUBLIC stm32cubemx)

target_include_directories(l76x_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Device
)