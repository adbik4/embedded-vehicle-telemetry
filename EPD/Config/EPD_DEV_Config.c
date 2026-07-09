/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-10-30
* | Info        :
# ******************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
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
#include "EPD_DEV_Config.h"
#include "stm32u3xx_hal_spi.h"

extern SPI_HandleTypeDef hspi1;
void DEV_SPI_WriteByte(UBYTE value)
{
    HAL_SPI_Transmit(&hspi1, &value, 1, 1000);
}

void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len)
{
    HAL_SPI_Transmit(&hspi1, value, len, 1000);
}

void DEV_GPIO_Mode(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UWORD Mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(Mode == 0) {
        GPIO_InitStruct.Pin = GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	} else {
		GPIO_InitStruct.Pin = GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
		// Debug (" %d OUT \r\n",Pin);
	}
}

void DEV_GPIO_Init()
{
    HAL_SPI_MspDeInit(&hspi1);
    
    //HAL_SPI_DeInit(&hspi1); 
//    __HAL_RCC_SPI1_CLK_DISABLE();
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}

void DEV_SPI_Init()
{
    HAL_SPI_MspInit(&hspi1);
    //HAL_SPI_DeInit(&hspi1); 
//    __HAL_RCC_SPI1_CLK_DISABLE();
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}


void DEV_SPI_SendData(UBYTE Reg)
{
	UBYTE i,j=Reg;
	DEV_GPIO_Mode(EINK_EPD_DIN_GPIO, 1);
    DEV_GPIO_Mode(EINK_EPD_SCLK_GPIO, 1);
	DEV_Digital_Write(EINK_EPD_CS_GPIO, 0);
	for(i = 0; i<8; i++)
    {
        DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 0);     
        if (j & 0x80)
        {
            DEV_Digital_Write(EINK_EPD_DIN_GPIO, 1);
        }
        else
        {
            DEV_Digital_Write(EINK_EPD_DIN_GPIO, 0);
        }
        
        DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 1);
        j = j << 1;
    }
	DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 0);
	DEV_Digital_Write(EINK_EPD_CS_GPIO, 1);
}

UBYTE DEV_SPI_ReadData()
{
	UBYTE i,j=0xff;
	DEV_GPIO_Mode(EINK_EPD_DIN_GPIO, 0);
    DEV_GPIO_Mode(EINK_EPD_SCLK_GPIO, 1);
	DEV_Digital_Write(EINK_EPD_CS_GPIO, 0);
	for(i = 0; i<8; i++)
	{
		DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 0);
		j = j << 1;
		if (DEV_Digital_Read(EINK_EPD_DIN_GPIO))
		{
            j = j | 0x01;
		}
		else
		{
            j= j & 0xfe;
		}
		DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 1);
	}
	DEV_Digital_Write(EINK_EPD_SCLK_GPIO, 0);
	DEV_Digital_Write(EINK_EPD_CS_GPIO, 1);
	return j;
}

int EPD_DEV_Module_Init(void)
{
    DEV_Digital_Write(EINK_EPD_DC_GPIO, 0);
    DEV_Digital_Write(EINK_EPD_CS_GPIO, 0);
	DEV_Digital_Write(EINK_EPD_PWR_GPIO, 1);
    DEV_Digital_Write(EINK_EPD_RST_GPIO, 1);
    return 0;
}

void EPD_DEV_Module_Exit(void)
{
    DEV_Digital_Write(EINK_EPD_DC_GPIO, 0);
    DEV_Digital_Write(EINK_EPD_CS_GPIO, 0);

    //close 5V
	DEV_Digital_Write(EINK_EPD_PWR_GPIO, 0);
    DEV_Digital_Write(EINK_EPD_RST_GPIO, 0);
}

