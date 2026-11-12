#include "gui.hpp"

#include "L76X.h"
#include "utilities.hpp"
#include "system_state.hpp"
#include <cstdio>
#include <optional>
#include <string>
#include <iostream>
#include <variant>
#include <type_traits>

#ifndef UNIT_TEST
    #include "ab_sensor_hub.h"
    #include "EPD_2in66.h"
    #include "EPD_DEV_Config.h"
    #include "GUI_Paint.h"
    #include "cmsis_os2.h"
    #include <stdlib.h>

    extern osMessageQueueId_t ComDebugQueueHandle;
#endif

extern SystemState system_state;

#ifndef UNIT_TEST
    extern Button button1;
    extern Button button2;
    extern Button button3;
#else
    extern ButtonMock button1;
    extern ButtonMock button2;
    extern ButtonMock button3;
#endif

namespace gui {

#if !defined(UNIT_TEST)
    #define IMAGE_SIZE ((EPD_2IN66_WIDTH % 8 == 0)? (EPD_2IN66_WIDTH / 8 ): (EPD_2IN66_WIDTH / 8 + 1)) * EPD_2IN66_HEIGHT
    static UBYTE Canvas[IMAGE_SIZE];
#endif

/* --------- GUI ---------- */

GUI::GUI() : currentScreen(&DebugScreen::getInstance()) {
    #if !defined(UNIT_TEST)
        if(EPD_DEV_Module_Init() != 0){
            PRINT_DEBUG("E-ink display initialisation failed.");
        }

        PRINT_DEBUG("e-Paper Init and Clear...\r\n");
        EPD_2IN66_Init(); // Full Refresh
        EPD_2IN66_Init_Partial(); // Configure the driver for fast refresh
        DEV_Delay_ms(500);

        Paint_NewImage(Canvas, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 90, WHITE);
    #endif
    currentScreen->init(this);
}

void GUI::setScreen(ScreenState* nextScreen) {
    currentScreen = nextScreen;
    currentScreen->init(this);
}

void GUI::processScreen() {
    currentScreen->processScreen(this);
}

void GUI::refreshScreen() {
    currentScreen->refresh(this);
}

/* ----- COMMON ----- */

#if !defined(UNIT_TEST)
    static void DrawGUIFrame(ScreenState* state) {
        static constexpr uint64_t BAR_HEIGHT = 130;
        Paint_DrawString_EN(60, BAR_HEIGHT, state->bottom_bar[0], &Font16, BLACK, WHITE);
        Paint_DrawString_EN(145, BAR_HEIGHT, state->bottom_bar[1], &Font16, BLACK, WHITE);
        Paint_DrawString_EN(230, BAR_HEIGHT, state->bottom_bar[2], &Font16, BLACK, WHITE);
    }

    static void DrawGUIList(const DispList& disp_list) {
        static constexpr uint64_t CONTENT_TOP_MARGIN = 40;
        static constexpr uint64_t CONTENT_L_MARGIN = 40;
        static constexpr uint64_t HEADER_L_MARGIN = 10;
        
        static constexpr uint64_t V_STEP = 20;
        static constexpr uint64_t CONTENT_BOTTOM = 130;

        uint64_t y_pos{0};
        for (const auto& variant : disp_list) {
            std::visit([&](const auto& paragraph) {
                if (paragraph.header.empty() || y_pos >= CONTENT_BOTTOM) {
                    return;
                }

                Paint_DrawString_EN(HEADER_L_MARGIN, y_pos, paragraph.header.c_str(), &Font20, BLACK, WHITE);
                y_pos += CONTENT_TOP_MARGIN;

                for(const auto& element : paragraph.contents) {
                    if (element.name.empty()) { continue; }
                    if (y_pos >= CONTENT_BOTTOM) { return; }

                    std::string line{};
                    line = element.name + ": " + std::to_string(element.value) + "\r\n";

                    Paint_DrawString_EN(CONTENT_L_MARGIN, y_pos, line.c_str(), &Font16, BLACK, WHITE);
                    y_pos += V_STEP;
                }

                y_pos += CONTENT_TOP_MARGIN;
            }, variant);
        }
    }
#endif

/* ----- DEBUG SCREEN ----- */

void DebugScreen::processScreen(GUI* gui) {
    ScreenState* nextScreen = &(gui->getCurrentScreen());

    /* Decide on the next state */
    if(button3.getState() == ButtonState::pressed) {
        nextScreen = &(MainScreen::getInstance());
    }

    /* Update screen if state has changed */
    if(nextScreen != &(gui->getCurrentScreen())) {
        gui->setScreen(nextScreen);
    }

    /* Clear button states */
    button1.goIdle();
    button2.goIdle();
    button3.goIdle();
}

void DebugScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Debug Screen...\r\n");

    #if !defined(UNIT_TEST)
        updateDispLists();

        Paint_Clear(WHITE);

        Paint_DrawString_EN(10, 0, "Debug Screen", &Font24, BLACK, WHITE);
        DrawGUIFrame(this);
        DrawGUIList(disp_lists[0]);
        
        EPD_2IN66_Display(Canvas);
        DEV_Delay_ms(500);
    #endif
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Debug Screen...\r\n");
}

void DebugScreen::updateDispLists() {
#if !defined(UNIT_TEST)
    const sensor_hub_data_t& imu_data = system_state.getIMUdata();

    disp_lists = {
        DispList{
            Paragraph<uint64_t, 4>{
                "System Time",
                {{{"timestamp", imu_data.timestamp_us}, {}, {}, {}}},
                1
            },
            Paragraph<float, 4>{
                "Linear Acceleration",
                {{{"acc_x", imu_data.mfx_6x.linear_acceleration[0]},
                  {"acc_y", imu_data.mfx_6x.linear_acceleration[1]},
                  {"acc_z", imu_data.mfx_6x.linear_acceleration[2]},
                  {}}},
                3
            },
            Paragraph<float, 4>{
                "Angular Rate",
                {{{"gyro_x", imu_data.angular_rate[0]},
                  {"gyro_y", imu_data.angular_rate[1]},
                  {"gyro_z", imu_data.angular_rate[2]},
                  {}}},
                3
            }
        },
        DispList{
            Paragraph<float, 4>{
                "Magnetic Field",
                {{{"mag_x", imu_data.magnetic_field[0]},
                  {"mag_y", imu_data.magnetic_field[1]},
                  {"mag_z", imu_data.magnetic_field[2]},
                  {}}},
                3
            },
            Paragraph<float, 4>{
                "Sensor Fusion",
                {{{"rot_a", imu_data.mfx_6x.quaternion[0]},
                  {"rot_b", imu_data.mfx_6x.quaternion[1]},
                  {"rot_c", imu_data.mfx_6x.quaternion[2]},
                  {"rot_d", imu_data.mfx_6x.quaternion[3]}}},
                4
            },
            Paragraph<uint64_t, 4>{"", {}, 0}
        }
    };
#endif
}

/* ----- MAIN SCREEN ----- */

void MainScreen::processScreen(GUI* gui) {
    ScreenState* nextScreen = &(gui->getCurrentScreen());
    
    /* Decide on the next state */
    if(button3.getState() == ButtonState::pressed) {
        nextScreen = &(StatsScreen::getInstance());
    }

    /* Update screen if state has changed */
    if(nextScreen != &(gui->getCurrentScreen())) {
        gui->setScreen(nextScreen);
    }

    /* Clear button states */
    button1.goIdle();
    button2.goIdle();
    button3.goIdle();
}

void MainScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Main Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    DrawGUIFrame(this);
    Paint_DrawString_EN(10, 0, "Main Screen", &Font24, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
    DEV_Delay_ms(500);
#endif
}

void MainScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Main Screen...\r\n");
}

/* ----- STATS SCREEN ----- */

void StatsScreen::processScreen(GUI* gui) {
    ScreenState* nextScreen = &(gui->getCurrentScreen());
    
    /* Decide on the next state */
    if(button3.getState() == ButtonState::pressed) {
        nextScreen = &(DebugScreen::getInstance());
    }

    /* Update screen if state has changed */
    if(nextScreen != &(gui->getCurrentScreen())) {
        gui->setScreen(nextScreen);
    }

    /* Clear button states */
    button1.goIdle();
    button2.goIdle();
    button3.goIdle();
}

void StatsScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Stats Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    DrawGUIFrame(this);
    Paint_DrawString_EN(10, 0, "Stats Screen", &Font24, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
    DEV_Delay_ms(500);
#endif
}

void StatsScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Stats Screen...\r\n");
}

} // namespace gui