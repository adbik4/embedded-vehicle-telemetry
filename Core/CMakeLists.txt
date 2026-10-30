add_library(telemetry_app_lib STATIC)

target_include_directories(telemetry_app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../Middlewares/Third_Party/CMSIS/RTOS2/Include
)

target_sources(telemetry_app_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/App/utilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/App/gui.cpp
)

if(TARGET stm32cubemx)
    target_link_libraries(telemetry_app_lib PUBLIC stm32cubemx)
endif()

if(TARGET epd_lib)
    target_link_libraries(telemetry_app_lib PUBLIC epd_lib)
endif()