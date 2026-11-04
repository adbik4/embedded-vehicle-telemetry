#include "gui.hpp"

#include "utilities.hpp"
#include "system_state.hpp"
#include <optional>

#if !defined(UNIT_TEST)
    #include "EPD_2in66.h"
    #include "EPD_DEV_Config.h"
    #include "GUI_Paint.h"
    #include "cmsis_os2.h"
    #include <stdlib.h>

    extern osMessageQueueId_t ComDebugQueueHandle;
#endif

extern SystemState system_state;

namespace gui {

#if !defined(UNIT_TEST)
    #define IMAGE_SIZE ((EPD_2IN66_WIDTH % 8 == 0)? (EPD_2IN66_WIDTH / 8 ): (EPD_2IN66_WIDTH / 8 + 1)) * EPD_2IN66_HEIGHT
    static UBYTE Canvas[IMAGE_SIZE];
#endif

/* --------- GUI ---------- */

GUI::GUI() : currentScreen(MainScreen::getInstance()) {
    #if !defined(UNIT_TEST)
        if(EPD_DEV_Module_Init() != 0){
            PRINT_DEBUG("E-ink display initialisation failed.");
        }

        PRINT_DEBUG("e-Paper Init and Clear...\r\n");
        EPD_2IN66_Init();
        EPD_2IN66_Clear();
        DEV_Delay_ms(500);

        Paint_NewImage(Canvas, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 90, WHITE);
    #endif
    currentScreen.init(this);
}

void GUI::setScreen(ScreenState* nextScreen) {
    currentScreen = *nextScreen;
    currentScreen.init(this);
}

void GUI::switchScreen() {
    currentScreen.switchScreen(this);
}

void GUI::refreshScreen() {
    currentScreen.refresh(this);
}

/* ----- DEBUG SCREEN ----- */

void DebugScreen::switchScreen(GUI* gui) {
    // TODO: implement
}

void DebugScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Debug Screen...\r\n");

    #if !defined(UNIT_TEST)
        Paint_Clear(WHITE);
        
        Paint_DrawString_EN(10, 0, "Debug Screen", &Font16, BLACK, WHITE);
        
        EPD_2IN66_Display(Canvas);
        DEV_Delay_ms(500);
    #endif
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Debug Screen...\r\n");
}

/* ----- MAIN SCREEN ----- */

void MainScreen::switchScreen(GUI* gui) {
    // TODO: implement
}

void MainScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Main Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    Paint_DrawString_EN(10, 0, "Main Screen", &Font16, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
    DEV_Delay_ms(500);
#endif
}

void MainScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Main Screen...\r\n");
}

/* ----- STATS SCREEN ----- */

void StatsScreen::switchScreen(GUI* gui) {
    // TODO: implement
}

void StatsScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Stats Screen...\r\n");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    Paint_DrawString_EN(10, 0, "Stats Screen", &Font16, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
    DEV_Delay_ms(500);
#endif
}

void StatsScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Stats Screen...\r\n");
}

} // namespace gui