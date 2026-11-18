#include "gui.hpp"

#include "L76X.h"
#include "utilities.hpp"
#include "system_state.hpp"
#include <cstdint>
#include <cstdio>
#include <optional>
#include <etl/string.h>
#include <etl/to_string.h>
#include <etl/format.h>
#include <etl/tuple.h>

#ifndef UNIT_TEST
    #include "ab_sensor_hub.h"
    #include "EPD_2in66.h"
    #include "EPD_DEV_Config.h"
    #include "GUI_Paint.h"
    #include "cmsis_os2.h"
    #include <stdlib.h>

    extern osMessageQueueId_t ComDebugQueueHandle;
#endif

#ifdef UNIT_TEST
using SystemState = MockSystemState;
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

void GUI::processInput() {
    currentScreen->processInput(this);
}

void GUI::refreshScreen() {
    currentScreen->refresh(this);
}

/* ----- COMMON ----- */

#if !defined(UNIT_TEST)
    static void GUI_DrawInputOptions(ScreenState* state) {
        static constexpr uint64_t BAR_HEIGHT = 130;
        Paint_DrawString_EN(60, BAR_HEIGHT, state->bottom_bar[0], &Font16, BLACK, WHITE);
        Paint_DrawString_EN(145, BAR_HEIGHT, state->bottom_bar[1], &Font16, BLACK, WHITE);
        Paint_DrawString_EN(230, BAR_HEIGHT, state->bottom_bar[2], &Font16, BLACK, WHITE);
    }

    static void DrawParagraph(const auto& paragraph, uint64_t& y_pos) {
        static constexpr uint64_t CONTENT_TOP_MARGIN = 18;
        static constexpr uint64_t CONTENT_L_MARGIN = 20;
        static constexpr uint64_t CONTENT_BOTTOM_MARGIN = 2;
        static constexpr uint64_t HEADER_L_MARGIN = 10;
        static constexpr uint64_t V_STEP = 15;

        if (paragraph.header.empty() || y_pos >= SCREEN_HEIGHT) {
            return;
        }

        Paint_DrawString_EN(HEADER_L_MARGIN, y_pos, paragraph.header.c_str(), &Font16, BLACK, WHITE);
        y_pos += CONTENT_TOP_MARGIN;

        for(const auto& element : paragraph.contents) {
            if (element.name.empty()) { continue; }
            if (y_pos >= SCREEN_HEIGHT) { return; }

            etl::string<128> line{element.name};
            line += ": ";
            if constexpr (is_etl_string<std::remove_cvref_t<decltype(element.value)>>::value) {
                line += element.value;
            } else {
                etl::to_string(
                    element.value,
                    line,
                    etl::format_spec().precision(3),
                    true
                );
            }

            Paint_DrawString_EN(CONTENT_L_MARGIN, y_pos, line.c_str(), &Font12, BLACK, WHITE);
            y_pos += V_STEP;
        }

        y_pos += CONTENT_BOTTOM_MARGIN;
    }

    template <typename... Paragraphs>
    static void GUI_DrawList(const etl::tuple<Paragraphs...>& disp_list) {
        uint64_t y_pos{0};

        etl::apply([&](const auto&... paragraphs) {
            (DrawParagraph(paragraphs, y_pos), ...);
        }, disp_list);
    }
#endif

/* ----- DEBUG SCREEN ----- */

void DebugScreen::update_pages() {
    SystemState& system_state = SystemState::getInstance();

    system_state.acquire();
    SystemState_t state = system_state.getState();

    etl::string<MAX_FONT12_CAPACITY> timestamp{}, g_time{};
    etl::format_to(timestamp, "{}", state.timestamp_us);
    etl::format_to(g_time, R"({} : {} : {} : {})",
        state.gnss.time.h,
        state.gnss.time.m,
        state.gnss.time.s,
        state.gnss.time.ms
    );

    etl::string<MAX_FONT12_CAPACITY> yaw{}, pitch{}, roll{};
    etl::format_to(yaw, "{:.3f}", state.mfx.sensor_fusion.rotation[0]);
    etl::format_to(pitch, "{:.3f}", state.mfx.sensor_fusion.rotation[1]);
    etl::format_to(roll, "{:.3f}", state.mfx.sensor_fusion.rotation[2]);

    etl::get<0>(_debug_page) = DebugPage1{
        Paragraph<etl::string<MAX_FONT12_CAPACITY>, 2> {
            "System Time",
            {{{"timestamp", timestamp},
            {"global time", g_time}}}
        },
        Paragraph<etl::string<MAX_FONT12_CAPACITY>, 4> {
            "Sensor Fusion",
            {{{"yaw", yaw},
            {"pitch", pitch},
            {"roll", roll},
            {"mag_cal", magcal_to_string<MAX_FONT12_CAPACITY>(state.mfx.mag_cal.cal_quality)}}}
        },
    };

    etl::get<1>(_debug_page) = DebugPage2{
        Paragraph<float_t, 3> {
            "Linear Acceleration",
            {{{"acc_x", state.mfx.sensor_fusion.linear_acceleration[0]},
            {"acc_y", state.mfx.sensor_fusion.linear_acceleration[1]},
            {"acc_z", state.mfx.sensor_fusion.linear_acceleration[2]}}}
        },
        Paragraph<float_t, 3> {
            "Angular Rate",
            {{{"gyro_x", state.angular_rate[0]},
            {"gyro_y", state.angular_rate[1]},
            {"gyro_z", state.angular_rate[2]}}}
        }
    };

    etl::string<MAX_FONT12_CAPACITY> lat{}, lon{}, speed{};
    etl::format_to(lat, R"({}* {}' {:.3f}" {})",
        state.gnss.coors.Lat.Degrees,
        state.gnss.coors.Lat.Minutes, 
        state.gnss.coors.Lat.Seconds, 
        state.gnss.coors.Lat.Area
    );
    etl::format_to(lon, R"({}* {}' {:.3f}" {})",
        state.gnss.coors.Lon.Degrees,
        state.gnss.coors.Lon.Minutes, 
        state.gnss.coors.Lon.Seconds, 
        state.gnss.coors.Lon.Area
    );
    etl::format_to(speed, "{:.3f}", state.gnss.speed);

    etl::get<2>(_debug_page) = DebugPage3{
        Paragraph<float_t, 3> {
            "Magnetic Field",
            {{{"mag_x", state.mag_field[0]},
            {"mag_y", state.mag_field[1]},
            {"mag_z", state.mag_field[2]}}}
        },
        Paragraph<etl::string<MAX_FONT12_CAPACITY>, 3> {
            "GNSS Coordinates",
            {{{"lat", lat},
            {"lon", lon},
            {"speed", speed}}}
        }
    };

    system_state.release();
}

void DebugScreen::increment_page() {
    if (_curr_page == PAGE_COUNT - 1) { return; }
    _curr_page++;
}

void DebugScreen::decrement_page() {
    if (_curr_page == 0) { return; }
    _curr_page--;
}

/* Note: This function is entirely AI generated as a workaround hack
    as I was unable to come up with a solution for this problem myself */
void DebugScreen::draw_current_page() {
#if !defined(UNIT_TEST)
    const auto draw_page = [this]<size_t PageIndex>(const auto& self) -> void {
        if constexpr (PageIndex == PAGE_COUNT) {
            configASSERT(false);
            return;
        } else {
            if (_curr_page == PageIndex) {
                GUI_DrawList(etl::get<PageIndex>(_debug_page));
                return;
            }

            self.template operator()<PageIndex + 1>(self);
        }
    };

    draw_page.template operator()<0>(draw_page);
#endif
}

void DebugScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Debug Screen...\r\n");

    DebugScreen::refresh(gui);
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Debug Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    GUI_DrawInputOptions(this);
#endif
#if !defined(UNIT_TEST)
    update_pages();
    draw_current_page();
#endif

#if !defined(UNIT_TEST)
    EPD_2IN66_Display(Canvas);
#endif
}

void DebugScreen::processInput(GUI* gui) {
    ScreenState* nextScreen = &(gui->getCurrentScreen());

    /* Decide on the next state */
    if(button1.getState() == ButtonState::pressed) {
        decrement_page();
    }
    if(button2.getState() == ButtonState::pressed) {
        increment_page();
    }
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

/* ----- MAIN SCREEN ----- */

void MainScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Main Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    GUI_DrawInputOptions(this);
    Paint_DrawString_EN(10, 0, "Main Screen", &Font24, BLACK, WHITE);
#endif
    
#if !defined(UNIT_TEST)
    EPD_2IN66_Display(Canvas);
#endif
}

void MainScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Main Screen...\r\n");
}

void MainScreen::processInput(GUI* gui) {
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

/* ----- STATS SCREEN ----- */

void StatsScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Stats Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    GUI_DrawInputOptions(this);
    Paint_DrawString_EN(10, 0, "Stats Screen", &Font24, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
#endif
}

void StatsScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Stats Screen...\r\n");
}

void StatsScreen::processInput(GUI* gui) {
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

} // namespace gui