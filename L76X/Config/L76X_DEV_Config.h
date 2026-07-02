#ifndef _L76X_DEV_CONFIG_H_
#define _L76X_DEV_CONFIG_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

#ifndef _EPD_DEV_CONFIG_H_
/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) HAL_GPIO_WritePin(_pin, _value == 0? GPIO_PIN_RESET:GPIO_PIN_SET)
#define DEV_Digital_Read(_pin) HAL_GPIO_ReadPin(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms)    HAL_Delay(__xms)
#endif

/**
 * L76X GPIO
**/
#define L76X_RX_GPIO     L76X_RX_GPIO_Port, L76X_RX_Pin
#define L76X_TX_GPIO     L76X_TX_GPIO_Port, L76X_TX_Pin
#define L76X_PPS_GPIO    L76X_PPS_GPIO_Port, L76X_PPS_Pin
#define L76X_RST_GPIO    L76X_RST_GPIO_Port, L76X_RST_Pin

/*
 * UART SETTINGS
*/
#define TRIES 5


/*-----------------------------------------------------------------------------*/
int L76X_DEV_Module_Init(void);
UBYTE DEV_Uart_ReceiveByte(void);
void DEV_Uart_SendByte(char data);
void DEV_Uart_SendString(char *data);
void DEV_Uart_ReceiveString(char *data, UWORD Num);

void DEV_Set_GPIOMode(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UWORD mode);
#endif

