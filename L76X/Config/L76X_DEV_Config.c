/*****************************************************************************
* | File        :   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2018-11-22
* | Info        :

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
#include "L76X_DEV_Config.h"
#include "stm32f4xx_hal_uart.h"

extern UART_HandleTypeDef huart5;

int L76X_DEV_Module_Init(void)
{
  DEV_Digital_Write(L76X_RST_GPIO, 1);
  return 0;
}

void DEV_Uart_WriteByte(UBYTE value)
{    
  HAL_UART_Transmit(&huart5, &value, 1, 1000);
}
/******************************************************************************
function: 
  Uart receiving and sending
******************************************************************************/
UBYTE DEV_Uart_ReceiveByte()
{
  UBYTE value;
  UBYTE count = 0;

  while(1){
    if(HAL_UART_Receive(&huart5, &value, 1, 1000) == HAL_OK){
      return value;
    }
    count++;
    if(count >= TRIES){
      //printf("Debug: ReceiveByte Timeout\r\n");
      //Error_Handler();
    }
  }
}

void DEV_Uart_SendByte(char data)
{
  HAL_UART_Transmit(&huart5, (uint8_t*)&data, 1, 1000);
}

void DEV_Uart_SendString(char *data)
{
  UWORD i;
  for(i=0; data[i] != '\0'; i++){
    HAL_UART_Transmit(&huart5, (uint8_t*)&data[i], 1, 1000);
  }
}

void DEV_Uart_ReceiveString(char *data, UWORD Num)
{  
  UWORD i = 0;
  UBYTE value = 0;
  UBYTE count = 0;

  while(1){
    if(HAL_UART_Receive(&huart5, &value, 1, 1000) == HAL_OK){
        data[i] =  value;
        i++;
        if(i >= Num){
            break;
          }
      }
    count++;
    if(count >= TRIES){
      //printf("Debug: ReceiveString Timeout\r\n");
      //Error_Handler();
    }
   }

  data[Num-1] = '\0';
}

void DEV_Set_GPIOMode(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UWORD mode)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(mode == 1) {
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
