#include "App/custom_algobuild.h"
#include "app_algobuild.h"

#include "algo_builder.h"
#include "ab_sensor_hub.h"
#include "app_mems_int_pin_a_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_UHCCR1_VAL  100 /* 10kHz/100 value */
#define DEFAULT_UHCCR2_VAL  200 /* 10kHz/50 value */
#define DEFAULT_UHCCR3_VAL  625 /* 10kHz/16 value */
#define DEFAULT_UHCCR4_VAL  400 /* 10kHz/25 value */

extern uint64_t StartTime;
extern sensor_hub_data_t sensor_hub_data;
extern uint32_t SensorReadRequest;

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

void AlgoBuilder_Init(void)
{
  /* Initialize cycle counter */
  DWT_Delay_Init();

  /* Initialize input with interrupt for DRDY signal */
  set_mems_int_pin_a_exti();

  /* Initialize sensors */
  BSP_SENSOR_ACC_Init();
  BSP_SENSOR_GYR_Init();
  BSP_SENSOR_MAG_Init();

  BSP_SENSOR_ACC_Enable();
  BSP_SENSOR_GYR_Enable();
  BSP_SENSOR_MAG_Enable();

  /* Blink with LED */
  BSP_LED_On(LED_RED);
  HAL_Delay(500);
  BSP_LED_Off(LED_RED);

  AB_Init();

  /* Check if UCF loading changed ODR or FS */
  FSM_MLC_Check_ODR_FS_Change();
}

/**
  * @brief  Handles the precise time
  * @param  Msg the time part of the stream
  * @retval None
  */
void Time_Handler(void)
{
  // TODO: replace with freeRTOS
  sensor_hub_data.timestamp_us = DWT_GetTickUS() - StartTime;
}

#ifdef __cplusplus
}
#endif