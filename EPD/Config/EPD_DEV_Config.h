/*****************************************************************************
* | File      	:   DEV_Config.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-10-30
* | Info        :
* 1.add:
*   UBYTE\UWORD\UDOUBLE
* 2.Change:
*   EPD_RST -> EINK_EPD_RST_GPIO
*   EPD_DC -> EINK_EPD_DC_GPIO
*   EPD_CS -> EINK_EPD_CS_GPIO
*   EPD_BUSY -> EINK_EPD_BUSY_GPIO
* 3.Remote:
*   EPD_RST_1\EPD_RST_0
*   EPD_DC_1\EPD_DC_0
*   EPD_CS_1\EPD_CS_0
*   EPD_BUSY_1\EPD_BUSY_0
* 3.add:
*   #define DEV_Digital_Write(_pin, _value) bcm2835_gpio_write(_pin, _value)
*   #define DEV_Digital_Read(_pin) bcm2835_gpio_lev(_pin)
*   #define DEV_SPI_WriteByte(__value) bcm2835_spi_transfer(__value)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef _EPD_DEV_CONFIG_H_
#define _EPD_DEV_CONFIG_H_

#include "main.h"
#include "stm32u3xx_hal.h"
#include "stm32u3xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * e-Paper GPIO
**/
#define EINK_EPD_RST_GPIO     EINK_RST_GPIO_Port, EINK_RST_Pin
#define EINK_EPD_DC_GPIO      EINK_DC_GPIO_Port, EINK_DC_Pin
#define EINK_EPD_PWR_GPIO     EINK_PWR_GPIO_Port, EINK_PWR_Pin
#define EINK_EPD_CS_GPIO      EINK_SPI_CS_GPIO_Port, EINK_SPI_CS_Pin
#define EINK_EPD_BUSY_GPIO    EINK_BUSY_GPIO_Port, EINK_BUSY_Pin
#define EINK_EPD_DIN_GPIO     EINK_DIN_GPIO_Port, EINK_DIN_Pin
#define EINK_EPD_SCLK_GPIO    EINK_SCK_GPIO_Port, EINK_SCK_Pin

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) HAL_GPIO_WritePin(_pin, _value == 0? GPIO_PIN_RESET:GPIO_PIN_SET)
#define DEV_Digital_Read(_pin) HAL_GPIO_ReadPin(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms) HAL_Delay(__xms);

void DEV_SPI_WriteByte(UBYTE value);
void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len);

int EPD_DEV_Module_Init(void);
void EPD_DEV_Module_Exit(void);
void DEV_GPIO_Init(void);
void DEV_SPI_Init(void);
void DEV_SPI_SendData(UBYTE Reg);
UBYTE DEV_SPI_ReadData(void);
#endif
