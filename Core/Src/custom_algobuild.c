#include "custom_algobuild.h"
#include "app_algobuild.h"

#include "bsp_ip_conf.h"
#include "algo_builder.h"
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

static void TIM_AL_Config(void);
static uint32_t DWT_Delay_Init(void);

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

  BSP_SENSOR_ACC_Enable();
  BSP_SENSOR_GYR_Enable();
  BSP_SENSOR_MAG_Enable();

  /* Blink with LED */
  BSP_LED_On(LED_RED);
  HAL_Delay(500);
  BSP_LED_Off(LED_RED);

  /* AlgoBuilder initialization */
  AB_Init();

  /* Check if UCF loading changed ODR or FS */
  FSM_MLC_Check_ODR_FS_Change();

  /* Start both the AlgoBuilder and Motion Library timers */
  TIM_AB_Start();
  TIM_AL_Start();
}

/**
  * @brief  Handles the precise time
  * @param  Msg the time part of the stream
  * @retval None
  */
void Time_Handler(void)
{
  sensor_hub_data.timestamp_us = DWT_GetTickUS() - StartTime;
}

#ifdef __cplusplus
}
#endif