#include "gui.hpp"

#include "utilities.hpp"
#include <optional>

#if !defined(UNIT_TEST)
    #include "EPD_2in66.h"
    #include "EPD_DEV_Config.h"
    #include "GUI_Paint.h"
    #include "cmsis_os2.h"
    #include <stdlib.h>

    extern osMessageQueueId_t ComDebugQueueHandle;
#endif

namespace gui {

#if !defined(UNIT_TEST)
    #define IMAGE_SIZE ((EPD_2IN66_WIDTH % 8 == 0)? (EPD_2IN66_WIDTH / 8 ): (EPD_2IN66_WIDTH / 8 + 1)) * EPD_2IN66_HEIGHT
    static UBYTE Canvas[IMAGE_SIZE];
#endif

/* --------- GUI ---------- */

GUI::GUI() : currentScreen(DebugScreen::getInstance()) {
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

void GUI::setScreen(ScreenState& nextScreen) {
    currentScreen = nextScreen;
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
    ScreenState& nextScreen = MainScreen::getInstance();
    gui->setScreen(nextScreen);
}

void DebugScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Debug Screen...");

    #if !defined(UNIT_TEST)
        Paint_Clear(WHITE);
        
        Paint_DrawString_EN(10, 0, "Debug Screen", &Font16, BLACK, WHITE);
        
        EPD_2IN66_Display(Canvas);
        // DEV_Delay_ms(2000);
    #endif
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Debug Screen...");
}

/* ----- MAIN SCREEN ----- */

void MainScreen::switchScreen(GUI* gui) {
    ScreenState& nextScreen = DebugScreen::getInstance();
    gui->setScreen(nextScreen);
}

void MainScreen::init(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Initialising Main Screen...");

#if !defined(UNIT_TEST)
    Paint_Clear(WHITE);

    Paint_DrawString_EN(10, 0, "Main Screen", &Font16, BLACK, WHITE);
    
    EPD_2IN66_Display(Canvas);
    // DEV_Delay_ms(2000);
#endif
}

void MainScreen::refresh(GUI* gui) {
    (void)gui;  // unused
    PRINT_DEBUG("Refreshing Main Screen...");
}

} // namespace gui