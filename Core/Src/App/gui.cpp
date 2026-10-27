#include "gui.hpp"

#include "cmsis_os2.h"
#include "EPD_DEV_Config.h"
#include "EPD_2in66.h"
#include "GUI_Paint.h"
#include "utilities.hpp"
#include <stdlib.h>

extern osMessageQueueId_t ComDebugQueueHandle;

GUI::GUI() {
    setScreen(DebugScreen::getInstance());

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
}

void GUI::setScreen(ScreenState& nextScreen) {
    currentScreen = &nextScreen;
    currentScreen->init(this);
}

void GUI::switchScreen() {
    currentScreen->switchScreen(this);
}

ScreenState& DebugScreen::switchScreen(GUI* gui) {
    ScreenState& nextScreen = *screenTransitions.at(this);
    gui->setScreen(nextScreen);
    return nextScreen;
}

void DebugScreen::init(GUI* gui) {
    (void)gui;
}

void DebugScreen::refresh(GUI* gui) {
    (void)gui;
}