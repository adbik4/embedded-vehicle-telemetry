cmake_minimum_required(VERSION 3.22)
project(gtest_ut LANGUAGES C CXX)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_BASE_DIR
    "${CMAKE_SOURCE_DIR}/deps"
    CACHE PATH "Persistent FetchContent dependency cache"
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(gtest_ut)

add_subdirectory(../Core Core)

target_include_directories(gtest_ut PRIVATE
    ../Middlewares/Third_Party/CMSIS/RTOS2/Include
)

target_sources(gtest_ut PRIVATE
    singleton_test.cpp
    gui_test.cpp
)

target_link_libraries(gtest_ut PRIVATE
    GTest::gtest_main
    telemetry_app_lib
)

include(GoogleTest)
gtest_discover_tests(gtest_ut)