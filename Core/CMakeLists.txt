add_library(telemetry_app_lib STATIC)

target_include_directories(telemetry_app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../Library/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../Library/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../X-CUBE-MEMS1/Target
)

target_sources(telemetry_app_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/App/custom_algobuild.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/App/custom_mems_control_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/App/utilities.c
)

target_link_libraries(telemetry_app_lib PUBLIC
    stm32cubemx
)