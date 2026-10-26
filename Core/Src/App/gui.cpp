#include "gui.hpp"

#include "cmsis_os2.h"
#include "EPD_DEV_Config.h"
#include "EPD_2in66.h"

extern osMessageQueueId_t ComDebugQueueHandle;

GUI::GUI() {
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
    }
    
    PRINT_DEBUG("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 270, WHITE);
    Paint_SetRotate(ROTATE_90);
}