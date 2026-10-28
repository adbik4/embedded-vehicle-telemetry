#include "gui.hpp"
#include "utilities.h"

#if defined(UNIT_TEST)
#include "cmsis_os2.h"
#include "EPD_DEV_Config.h"
#include "EPD_2in66.h"
#include "GUI_Paint.h"
#include <stdlib.h>

extern osMessageQueueId_t ComDebugQueueHandle;
#endif

/* --------- GUI ---------- */

GUI::GUI() : currentScreen(DebugScreen::getInstance()) {
#if defined(UNIT_TEST)

    if(EPD_DEV_Module_Init() != 0){
        PRINT_DEBUG("E-ink display initialisation failed.");
    }

    PRINT_DEBUG("e-Paper Init and Clear...\r\n");
    EPD_2IN66_Init();
    EPD_2IN66_Clear();
    DEV_Delay_ms(500);

    //Create a new image cache 
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2IN66_WIDTH % 8 == 0)? (EPD_2IN66_WIDTH / 8 ): (EPD_2IN66_WIDTH / 8 + 1)) * EPD_2IN66_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        PRINT_DEBUG("Failed to apply for black memory...\r\n");
        return;
    }
    
    PRINT_DEBUG("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 270, WHITE);
    Paint_SetRotate(ROTATE_90);
#endif
}

void GUI::setScreen(ScreenState& nextScreen) {
    currentScreen = nextScreen;
    currentScreen.init(this);
}

void GUI::switchScreen() {
    currentScreen.switchScreen(this);
}

void GUI::refreshScreen() {
    currentScreen->refresh(this);
}

/* ----- DEBUG SCREEN ----- */

void DebugScreen::switchScreen(GUI* gui) {
    ScreenState& nextScreen = MainScreen::getInstance();
    gui->setScreen(nextScreen);
}

void DebugScreen::init(GUI* gui) {
    (void)gui;
    PRINT_DEBUG("Initialising Debug Screen...")
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;
    PRINT_DEBUG("Refreshing Debug Screen...")
}

/* ----- MAIN SCREEN ----- */

void MainScreen::switchScreen(GUI* gui) {
    ScreenState& nextScreen = DebugScreen::getInstance();
    gui->setScreen(nextScreen);
}

void MainScreen::init(GUI* gui) {
    (void)gui;
    PRINT_DEBUG("Initialising Main Screen...")
}

void MainScreen::refresh(GUI* gui) {
    (void)gui;
    PRINT_DEBUG("Refreshing Main Screen...")
}