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
extern uint32_t SensorReadRequest;

static uint32_t DWT_Delay_Init(void);

/**
  * @brief  Measure ODR of the sensor
  * @param  None
  * @retval 1 in case of success 0 otherwise
  */
static int32_t meas_odr(float_t *odr_measured)
{
  uint8_t  odr_meas_enable = 1;
  uint16_t odr_meas_iter = 0;
  uint16_t odr_meas_start_time = 0;
  uint16_t odr_meas_stop_time = 0;
  uint16_t odr_meas_samples = 150; /* number of measured samples for calculating ODR */
  volatile uint32_t start = 0;

  /* Set DRDY pulsed mode */
  BSP_SENSOR_ACC_SetDRDYMode(1U);

  start = HAL_GetTick();

  while (odr_meas_enable != 0U)
  {
    if ((HAL_GetTick() - start) > 100000U)
    {
      /* Timeout */
      return 1;
    }

    /* Get start time */
    if (odr_meas_iter == 0U)
    {
      odr_meas_start_time = (uint16_t)HAL_GetTick();
    }

    /* Get stop time */
    if (odr_meas_iter == (odr_meas_samples - 1U))
    {
      odr_meas_stop_time = (uint16_t)HAL_GetTick();
      odr_meas_enable = 0;
    }

    /* Stop after measuring "odr_meas_samples" values */
    if (odr_meas_iter < odr_meas_samples)
    {
      odr_meas_iter++;
    }
  }

  /* Calculate measured ODR */
  *odr_measured = (1000.0f * (float_t)odr_meas_samples) / ((float_t)odr_meas_stop_time - (float_t)odr_meas_start_time);

  /* Set DRDY latched mode */
  BSP_SENSOR_ACC_SetDRDYMode(0U);

  return 0;
}

/**
  * @brief  Set accelerometer full scale
  */
static void Set_Accelero_FS(uint32_t accel_fs)
{
  int32_t fs;

  switch (accel_fs)
  {
    case 0:
      fs =  2;
      break;
    case 1:
      fs =  4;
      break;
    case 2:
      fs =  8;
      break;
    case 3:
      fs = 16;
      break;
    default:
      fs = 4;
      break;
  }

  BSP_SENSOR_ACC_SetFullScale(fs);
}

/**
  * @brief  Set gyroscope full scale
  */
static void Set_Gyro_FS(uint32_t gyro_fs)
{
  int32_t fs;

  switch (gyro_fs)
  {
    case 0:
      fs =  245;
      break;
    case 1:
      fs =  500;
      break;
    case 2:
      fs = 1000;
      break;
    case 3:
      fs = 2000;
      break;
    default:
      fs = 500;
      break;
  }

  BSP_SENSOR_GYR_SetFullScale(fs);
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
  // TODO: replace with freeRTOS
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

  SensorHub_Init(DRC_ACCELEROMETER, 100, 1, 1);

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
  sensor_hub_data.timestamp_us = DWT_GetTickUS() - StartTime;
}

/**
  * @brief  SensorHub initialization, set data polling period and full scales
  */
void SensorHub_Init(uint32_t data_rate_control, uint32_t data_rate, uint32_t accel_fs, uint32_t gyro_fs)
{
  sensor_hub_data.data_rate_control = (data_rate_control_t)data_rate_control;

  if ((data_rate_control_t)data_rate_control == DRC_OFFLINE)
  {
    sensor_hub_data.data_rate_Hz = data_rate;
    sensor_hub_data.meas_data_rate_Hz = data_rate;
    sensor_hub_data.iteration_counter = 0;

    offline_data.timestamp_ms = 0;
  }
  else
  {
    float_t real_odr = 0.0f;

    sensor_hub_data.motion_ac_enable = 0;
    sensor_hub_data.motion_aw_enable = 0;
    sensor_hub_data.motion_ec_enable = 0;
    sensor_hub_data.motion_fx_6x_enable = 0;
    sensor_hub_data.motion_fx_9x_enable = 0;
    sensor_hub_data.motion_gc_enable = 0;
    sensor_hub_data.motion_id_enable = 0;
    sensor_hub_data.motion_mc_enable = 0;
    sensor_hub_data.motion_pm_enable = 0;
    sensor_hub_data.motion_pw_enable = 0;
    sensor_hub_data.motion_tl_enable = 0;

    sensor_hub_data.data_rate_Hz = data_rate;

    switch (data_rate_control)
    {
      case DRC_TIMER:
        TIM_AB_Config(data_rate);
        BSP_SENSOR_ACC_SetDRDYInt(0U);
        BSP_SENSOR_GYR_SetDRDYInt(0U);
        break;

      case DRC_ACCELEROMETER:
        sensor_hub_data.acceleration_enable = 1;
        BSP_SENSOR_ACC_SetDRDYInt(1U);
        BSP_SENSOR_GYR_SetDRDYInt(0U);
        break;

      case DRC_GYROSCOPE:
        sensor_hub_data.angular_rate_enable = 1;
        BSP_SENSOR_ACC_SetDRDYInt(0U);
        BSP_SENSOR_GYR_SetDRDYInt(1U);
        break;

      default:
        break;
    }

    Set_Accelero_FS(accel_fs);
    Set_Gyro_FS(gyro_fs);

    BSP_SENSOR_ACC_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);
    BSP_SENSOR_GYR_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);
    BSP_SENSOR_MAG_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);
    BSP_SENSOR_PRESS_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);
    BSP_SENSOR_TEMP_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);
    BSP_SENSOR_HUM_SetOutputDataRate((float_t)sensor_hub_data.data_rate_Hz);

    if (((data_rate_control_t)data_rate_control == DRC_ACCELEROMETER)
        || ((data_rate_control_t)data_rate_control == DRC_GYROSCOPE))
    {
      if (meas_odr(&real_odr) == 1U) { sensor_hub_data.meas_data_rate_Hz = (uint32_t) real_odr; }
    }
    else
    {
      sensor_hub_data.meas_data_rate_Hz = data_rate;
    }

    sensor_hub_data.iteration_counter = 0;
  }
}

#ifdef __cplusplus
}
#endif