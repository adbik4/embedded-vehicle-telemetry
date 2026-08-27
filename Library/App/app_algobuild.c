/**
  ******************************************************************************
  * @file    app_algobuild.c
  * @author  MEMS Software Solutions Team
  * @brief   This file provides code for the configuration of the STMicroelectronics.X-CUBE-ALGOBUILD.1.5.0 instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_algobuild.h"
#include "main.h"
#include <stdio.h>

#include "bsp_ip_conf.h"
#include "algo_builder.h"
#include "com.h"
#include "app_mems_int_pin_a_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEFAULT_UHCCR1_VAL  100 /* 10kHz/100 value */
#define DEFAULT_UHCCR2_VAL  200 /* 10kHz/50 value */
#define DEFAULT_UHCCR3_VAL  625 /* 10kHz/16 value */
#define DEFAULT_UHCCR4_VAL  400 /* 10kHz/25 value */

#define INIT_ERROR_QUEUE_SIZE 4

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TMsg MsgDat;

/* Global variables ----------------------------------------------------------*/
extern sensor_hub_data_t sensor_hub_data;
extern offline_data_t offline_data;

uint64_t StartTime = 0;

volatile uint8_t DataLoggerStatusChanged = 0;
volatile uint32_t SensorReadRequest = 0;
volatile uint32_t Update16Hz = 0;
volatile uint32_t Update25Hz = 0;
volatile uint32_t Update50Hz = 0;
volatile uint32_t Update100Hz = 0;

volatile int32_t Message_Length;

uint8_t DataLoggerActive = 0;

#define FW_VERSION "12.0.0"

#define BOARD_NAME "CUSTOM"

uint8_t PresentationString[] = {"MEMS shield demo,201,"FW_VERSION",0.0.0,"BOARD_NAME};
uint8_t PresentationStringLen = sizeof(PresentationString);

/* Private function prototypes -----------------------------------------------*/
static void AlgoBuilder_Init(void);
static void AlgoBuilder_Process(void);

static void TIM_AL_Config(void);
static void Time_Handler(TMsg *Msg);

static uint32_t DWT_Delay_Init(void);

/* Functions Definition ------------------------------------------------------*/

void MX_Library_Init(void)
{
  /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN Library_Init_PreTreatment */

  /* USER CODE END Library_Init_PreTreatment */
  AlgoBuilder_Init();
  /* USER CODE BEGIN Library_Init_PostTreatment */

  /* USER CODE END Library_Init_PostTreatment */
}

/*
 * LM background task
 */
void MX_Library_Process(void)
{
  /* USER CODE BEGIN Library_Process_PreTreatment */

  /* USER CODE END Library_Process_PreTreatment */
  AlgoBuilder_Process();
  /* USER CODE BEGIN Library_Process_PostTreatment */

  /* USER CODE END Library_Process_PostTreatment */
}

/**
  * @brief  Initialize the AlgoBuilder application
  * @retval None
  */
void AlgoBuilder_Init(void)
{
  /* Initialize push button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /* Initialize selected USART */
  COM_InitTypeDef huart1;
  huart1.BaudRate = 921600;
  huart1.WordLength = UART_WORDLENGTH_8B;
  huart1.StopBits = UART_STOPBITS_1;
  huart1.Parity = UART_PARITY_NONE;
  huart1.HwFlowCtl = UART_HWCONTROL_NONE;
  BSP_COM_Init(COM1, &huart1);

  /* Initialize timer for AlgoBuilder process synchronization */
  BSP_IP_TIM_INIT_AB();

  /* Initialize timer for motion libraries synchronization */
  BSP_IP_TIM_INIT_AL();
  TIM_AL_Config();

  /* Initialize cycle counter */
  DWT_Delay_Init();

  /* Initialize input with interrupt for DRDY signal */
  set_mems_int_pin_a_exti();

  /* Initialize sensors */
  BSP_SENSOR_ACC_Init();
  BSP_SENSOR_GYR_Init();
  BSP_SENSOR_MAG_Init();
  BSP_SENSOR_PRESS_Init();
  BSP_SENSOR_TEMP_Init();
  BSP_SENSOR_HUM_Init();

  BSP_SENSOR_ACC_Enable();
  BSP_SENSOR_GYR_Enable();
  BSP_SENSOR_MAG_Enable();
  BSP_SENSOR_PRESS_Enable();
  BSP_SENSOR_TEMP_Enable();
  BSP_SENSOR_HUM_Enable();

  /* Blink with LED */
  BSP_LED_On(LED_RED);
  HAL_Delay(500);
  BSP_LED_Off(LED_RED);

  /* Start receiving data from USART */
  StartReceiveMsg();

  /* AlgoBuilder initialization */
  AB_Init();

  /* Check if UCF loading changed ODR or FS */
  FSM_MLC_Check_ODR_FS_Change();

  /* To start reading in case of interrupt driven flow */
  SensorReadRequest = 1;

  /* Ensure that User Button pressing will be evaluated from this point */
  DataLoggerActive = 0;
  DataLoggerStatusChanged = 0;
}

/**
  * @brief  Process of the AlgoBuilder application
  * @retval None
  */
void AlgoBuilder_Process(void)
{
  TMsg MsgCmd;

  /* Process incoming data */
  if ((ReceivedMSG((TMsg *)&MsgCmd)) && (MsgCmd.Data[0] == DEV_ADDR))
  {
    HandleMSG((TMsg *)&MsgCmd);
  }

  /* Handle user button press */
  if (DataLoggerStatusChanged != 0)
  {
    if (DataLoggerActive == 1)
    {
      TIM_AB_Start();
      TIM_AL_Start();
    }
    else
    {
      TIM_AB_Stop();
      TIM_AL_Stop();
    }
    MsgCmd.Data[0] = 1;
    MsgCmd.Data[1] = DEV_ADDR;
    MsgCmd.Data[2] = CMD_Get_Streaming_Status + CMD_Reply_Add;
    MsgCmd.Data[3] = DataLoggerActive;
    MsgCmd.Len = 4;
    SendMsg(&MsgCmd);

    DataLoggerStatusChanged = 0;
  }

  /* Execute AlgoBuilder Handler and send data stream */
  if (SensorReadRequest && DataLoggerActive)
  {
    SensorReadRequest = 0;

    Time_Handler(&MsgDat);

    BSP_LED_On(LED_RED);
    AB_Handler();
    BSP_LED_Off(LED_RED);

    INIT_STREAMING_HEADER(&MsgDat);
    MsgDat.Len += Message_Length + 6;

    SendMsg(&MsgDat);
  }
}

/**
  * @brief  TIM_AB config function
  * @param  None
  * @retval None
  * @details This function initialize the Timer used to synchronize the AB algorithm
  */
void TIM_AB_Config(uint32_t freq)
{
  uint32_t tim_counter_clock;

  if (SystemCoreClock > 120000000)
  {
    tim_counter_clock = 4000; /* TIM counter clock 4 kHz */
  }
  else
  {
    tim_counter_clock = 2000; /* TIM counter clock 2 kHz */
  }
  uint32_t prescaler_value = (uint32_t)((SystemCoreClock / tim_counter_clock) - 1);
  uint32_t period = (tim_counter_clock / freq) - 1;

  BSP_IP_TIM_HANDLE_AB.Init.Prescaler = prescaler_value;
  BSP_IP_TIM_HANDLE_AB.Init.CounterMode = TIM_COUNTERMODE_UP;
  BSP_IP_TIM_HANDLE_AB.Init.Period = period;
  BSP_IP_TIM_HANDLE_AB.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  BSP_IP_TIM_HANDLE_AB.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&BSP_IP_TIM_HANDLE_AB) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  TIM_AB start timer
  * @param  None
  * @retval None
  */
void TIM_AB_Start(void)
{
  HAL_TIM_Base_Start_IT(&BSP_IP_TIM_HANDLE_AB);
}

/**
  * @brief  TIM_AB stop timer
  * @param  None
  * @retval None
  */
void TIM_AB_Stop(void)
{
  HAL_TIM_Base_Stop_IT(&BSP_IP_TIM_HANDLE_AB);
}

/**
  * @brief  TIM_AL config function
  * @param  None
  * @retval None
  * @details This function initialize the timer used to synchronize the enabled algorithms
  */
void TIM_AL_Config(void)
{
  uint32_t prescaler_value;
  TIM_OC_InitTypeDef sConfig;

  /* Compute the prescaler value to have TIM1 counter clock equal to 10 KHz */
  prescaler_value = (uint32_t)((SystemCoreClock / 10000) - 1);

  BSP_IP_TIM_HANDLE_AL.Init.Prescaler = prescaler_value;
  BSP_IP_TIM_HANDLE_AL.Init.CounterMode   = TIM_COUNTERMODE_UP;
  BSP_IP_TIM_HANDLE_AL.Init.Period = 0xFFFF;
  BSP_IP_TIM_HANDLE_AL.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_OC_Init(&BSP_IP_TIM_HANDLE_AL);

  /* Configure the Output Compare channels */
  /* Common configuration for all channels */
  sConfig.OCMode     = TIM_OCMODE_TOGGLE;
  sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

  /* Output Compare Toggle Mode configuration: Channel1 */
  sConfig.Pulse = DEFAULT_UHCCR1_VAL;
  HAL_TIM_OC_ConfigChannel(&BSP_IP_TIM_HANDLE_AL, &sConfig, TIM_CHANNEL_1);

  /* Output Compare Toggle Mode configuration: Channel2 */
  sConfig.Pulse = DEFAULT_UHCCR2_VAL;
  HAL_TIM_OC_ConfigChannel(&BSP_IP_TIM_HANDLE_AL, &sConfig, TIM_CHANNEL_2);

  /* Output Compare Toggle Mode configuration: Channel3 */
  sConfig.Pulse = DEFAULT_UHCCR3_VAL;
  HAL_TIM_OC_ConfigChannel(&BSP_IP_TIM_HANDLE_AL, &sConfig, TIM_CHANNEL_3);

  /* Output Compare Toggle Mode configuration: Channel4 */
  sConfig.Pulse = DEFAULT_UHCCR4_VAL;
  HAL_TIM_OC_ConfigChannel(&BSP_IP_TIM_HANDLE_AL, &sConfig, TIM_CHANNEL_4);
}

/**
  * @brief  TIM_AL start timer
  * @param  None
  * @retval None
  */
void TIM_AL_Start(void)
{
  HAL_TIM_OC_Start_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_1);
  HAL_TIM_OC_Start_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_2);
  HAL_TIM_OC_Start_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_3);
  HAL_TIM_OC_Start_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_4);
}

/**
  * @brief  TIM_AL stop timer
  * @param  None
  * @retval None
  */
void TIM_AL_Stop(void)
{
  HAL_TIM_OC_Stop_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_1);
  HAL_TIM_OC_Stop_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_2);
  HAL_TIM_OC_Stop_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_3);
  HAL_TIM_OC_Stop_IT(&BSP_IP_TIM_HANDLE_AL, TIM_CHANNEL_4);
}

/**
  * @brief  Handles the precise time
  * @param  Msg the time part of the stream
  * @retval None
  */
static void Time_Handler(TMsg *Msg)
{
  uint64_t time_us;

  if (sensor_hub_data.data_rate_control == DRC_OFFLINE)
  {
    memcpy(&Msg->Data[3], &offline_data.timestamp_us, 6);
    sensor_hub_data.timestamp_us = offline_data.timestamp_us;
  }
  else
  {
    time_us = DWT_GetTickUS() - StartTime;
    memcpy(&Msg->Data[3], &time_us, 6);
    sensor_hub_data.timestamp_us = time_us;
  }
}

/**
  * @brief  Configures the current time and date
  * @param  hh the hour value to be set
  * @param  mm the minute value to be set
  * @param  ss the second value to be set
  * @retval None
  */
void RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss)
{
  RTC_TimeTypeDef stimestructure;

  stimestructure.Hours = hh;
  stimestructure.Minutes = mm;
  stimestructure.Seconds = ss;
  stimestructure.SubSeconds = 0;
  stimestructure.TimeFormat     = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&hrtc, &stimestructure, FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  BSP Push Button callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_KEY)
  {
    if (BSP_PB_GetState(BUTTON_KEY) == (uint32_t)GPIO_PIN_RESET)
    {
      if (DataLoggerStatusChanged == 0)
      {
        if (DataLoggerActive == 0)
        {
          DataLoggerActive = 1;
        }
        else
        {
          DataLoggerActive = 0;
        }
        DataLoggerStatusChanged = 1;
      }
    }
  }
}

/**
  * @brief  Period elapsed callback
  * @param  htim pointer to a TIM_HandleTypeDef structure that contains
  *              the configuration information for TIM module.
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == BSP_IP_TIM_HANDLE_AB.Instance)
  {
    SensorReadRequest = 1;
  }
}

/**
  * @brief  Output Compare callback in non blocking mode
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t uhCapture = 0;

  if (htim->Instance == BSP_IP_TIM_HANDLE_AL.Instance)
  {
    /* TIM_CH1 toggling with frequency = 100Hz */
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (uhCapture + DEFAULT_UHCCR1_VAL));
      Update100Hz = 1;
    }

    /* TIM_CH2 toggling with frequency = 50Hz */
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (uhCapture + DEFAULT_UHCCR2_VAL));
      Update50Hz = 1;
    }

    /* TIM_CH3 toggling with frequency = 16Hz */
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
      uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (uhCapture + DEFAULT_UHCCR3_VAL));
      Update16Hz = 1;
    }

    /* TIM_CH4 toggling with frequency = 25 Hz */
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
    {
      uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, (uhCapture + DEFAULT_UHCCR4_VAL));
      Update25Hz = 1;
    }
  }
}

/**
  * @brief  Initializes DWT_Clock_Cycle_Count for DWT_GetTickUS function
  * @retval Error DWT counter (1: clock cycle counter not started, 0: clock cycle counter works)
  */
static uint32_t DWT_Delay_Init(void)
{
  /* Disable TRC */
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;

  /* Enable TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Disable clock cycle counter */
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;

  /* Enable clock cycle counter */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  /* Reset the clock cycle counter value */
  DWT->CYCCNT = 0;

  /* Delay 1ms */
  HAL_Delay(1);

  /* Check if clock cycle counter has started */
  if (DWT->CYCCNT)
  {
    return 0; /* Clock cycle counter started */
  }
  else
  {
    return 1; /* Clock cycle counter not started */
  }
}

/**
  * @brief  Get relative time in micro seconds
  * @note   Call at least every 2^32 cycles. Do not call from interrupt context!
  * @retval Relative time in micro seconds
  */
uint64_t DWT_GetTickUS(void)
{
  static uint64_t last_cycle_count_64 = 0;
  uint32_t clock_MHz = HAL_RCC_GetHCLKFreq() / 1000000;

  last_cycle_count_64 += DWT->CYCCNT - (uint32_t)(last_cycle_count_64);

  return (uint64_t)(last_cycle_count_64 / clock_MHz);
}

#ifdef __cplusplus
}
#endif
