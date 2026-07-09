cmake_minimum_required(VERSION 3.22)

add_library(epd_lib STATIC
    Config/EPD_DEV_Config.c
    Display/EPD_2in66.c
    Examples/EPD_2in66_test.c
    Examples/ImageData.c
    Examples/ImageData2.c
    Fonts/font8.c
    Fonts/font12.c
    Fonts/font12CN.c
    Fonts/font16.c
    Fonts/font20.c
    Fonts/font24.c
    Fonts/font24CN.c
    GUI/GUI_Paint.c
)

target_link_libraries(epd_lib PUBLIC
    stm32cubemx
)

target_include_directories(epd_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Display
    ${CMAKE_CURRENT_SOURCE_DIR}/Examples
    ${CMAKE_CURRENT_SOURCE_DIR}/Fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/GUI
)