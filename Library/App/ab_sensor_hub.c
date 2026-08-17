/**
  ******************************************************************************
  * @file    ab_sensor_hub.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder sensor hub
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ab_sensor_hub.h"
#include "ab_fsm_mlc.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_SensorHub
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define FX_STATE_SIZE (size_t)(2432)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern uint32_t SensorReadRequest;
extern offline_data_t offline_data;

extern volatile uint32_t Update16Hz;
extern volatile uint32_t Update25Hz;
extern volatile uint32_t Update50Hz;
extern volatile uint32_t Update100Hz;

extern void *ACCELERO_handle;
extern void *GYRO_handle;
extern void *MAGNETO_handle;
extern void *HUMIDITY_handle;
extern void *TEMPERATURE_handle;
extern void *PRESSURE_handle;

sensor_hub_data_t sensor_hub_data;

#if (MotionFX_9X)
static uint8_t mfxstate_9x[FX_STATE_SIZE];
#endif

#if (MotionFX_6X)
static uint8_t mfxstate_6x[FX_STATE_SIZE];
#endif

float_t acc_odr_before_ucf;
uint32_t acc_fs_before_ucf;
float_t gyro_odr_before_ucf;
uint32_t gyro_fs_before_ucf;

/* Private function prototypes -----------------------------------------------*/
static void Set_Accelero_FS(uint32_t accel_fs);
static void Set_Gyro_FS(uint32_t gyro_fs);
static float_t Get_Accelero_ODR(void);
static uint32_t Get_Accelero_FS(void);
static float_t Get_Gyro_ODR(void);
static uint32_t Get_Gyro_FS(void);

#if (MotionEC)
static void Create_Rotation_Matrix(const char *orientation, float_t *matrix);
static void Axis_Transformation(float_t *o, float_t *i, float_t *matrix);
#endif /* MotionEC */

#if (MotionAC)
static void MotionAC_Update_Data(void);
#endif /* MotionAC */
#if (MotionAW)
static void MotionAW_Update_Data(void);
#endif /* MotionAW */
#if (MotionEC)
static void MotionEC_Update_Data(void);
#endif /* MotionEC */
#if (MotionFX_9X)
static void MotionFX_9X_Update_Data(void);
#endif /* MotionFX_9X */
#if (MotionFX_6X)
static void MotionFX_6X_Update_Data(void);
#endif /* MotionFX_6X */
#if (MotionGC)
static void MotionGC_Update_Data(void);
#endif /* MotionGC */
#if (MotionID)
static void MotionID_Update_Data(void);
#endif /* MotionID */
#if (MotionMC)
static void MotionMC_Update_Data(void);
#endif /* MotionMC */
#if (MotionPM)
static void MotionPM_Update_Data(void);
#endif /* MotionPM */
#if (MotionPW)
static void MotionPW_Update_Data(void);
#endif /* MotionPW */
#if (MotionTL)
static void MotionTL_Update_Data(void);
#endif /* MotionTL */

/* Functions Definition ------------------------------------------------------*/
static uint8_t meas_odr(float_t *odr_measured);

/**
  * @brief  Measure ODR of the sensor
  * @param  None
  * @retval 1 in case of success 0 otherwise
  */
static uint8_t meas_odr(float_t *odr_measured)
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
      return 0;
    }

    if (SensorReadRequest == 1U)
    {
      SensorReadRequest = 0;

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
  }

  /* Calculate measured ODR */
  *odr_measured = (1000.0f * (float_t)odr_meas_samples) / ((float_t)odr_meas_stop_time - (float_t)odr_meas_start_time);

  /* Set DRDY latched mode */
  BSP_SENSOR_ACC_SetDRDYMode(0U);

  return 1;
}

/**
  * @brief  SensorHub initialization, set data polling period and full scales
  */
void Sensor_Hub_Init(uint32_t data_rate_control, uint32_t data_rate, uint32_t accel_fs, uint32_t gyro_fs)
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

/**
  * @brief  SensorHub handler, read data from sensors
  */
void Sensor_Hub_Handler(void **pdata)
{
  *pdata = (void *) &sensor_hub_data;

  if (sensor_hub_data.data_rate_control == DRC_OFFLINE)
  {
    /* Accelerometer Data */
    if (sensor_hub_data.acceleration_enable == 1U)
    {
      sensor_hub_data.acceleration[0] = (float_t)offline_data.acceleration_x_mg * FROM_MG_TO_G;
      sensor_hub_data.acceleration[1] = (float_t)offline_data.acceleration_y_mg * FROM_MG_TO_G;
      sensor_hub_data.acceleration[2] = (float_t)offline_data.acceleration_z_mg * FROM_MG_TO_G;
    }

    /* Gyroscope Data */
    if (sensor_hub_data.angular_rate_enable == 1U)
    {
      sensor_hub_data.angular_rate[0] = (float_t)offline_data.angular_rate_x_mdps * FROM_MDPS_TO_DPS;
      sensor_hub_data.angular_rate[1] = (float_t)offline_data.angular_rate_y_mdps * FROM_MDPS_TO_DPS;
      sensor_hub_data.angular_rate[2] = (float_t)offline_data.angular_rate_z_mdps * FROM_MDPS_TO_DPS;
    }

    /* Magnetometer Data */
    if (sensor_hub_data.magnetic_field_enable == 1U)
    {
      sensor_hub_data.magnetic_field[0] = (float_t)offline_data.magnetic_field_x_mgauss * FROM_MGAUSS_TO_UT;
      sensor_hub_data.magnetic_field[1] = (float_t)offline_data.magnetic_field_y_mgauss * FROM_MGAUSS_TO_UT;
      sensor_hub_data.magnetic_field[2] = (float_t)offline_data.magnetic_field_z_mgauss * FROM_MGAUSS_TO_UT;
    }

    /* Pressure Sensor Data */
    if (sensor_hub_data.pressure_enable == 1U)
    {
      sensor_hub_data.pressure = offline_data.pressure;
    }

    /* Humidity Sensor Data */
    if (sensor_hub_data.humidity_enable == 1U)
    {
      sensor_hub_data.humidity = offline_data.humidity;
    }

    /* Temperature Sensor Data */
    if (sensor_hub_data.temperature_enable == 1U)
    {
      sensor_hub_data.temperature = offline_data.temperature;
    }
  }
  else
  {
    /* Accelerometer Data */
    if (sensor_hub_data.acceleration_enable == 1U)
    {
      BSP_SENSOR_ACC_GetAxes(&sensor_hub_data.acc);
      sensor_hub_data.acceleration[0] = (float_t)(&sensor_hub_data.acc)->x * FROM_MG_TO_G;
      sensor_hub_data.acceleration[1] = (float_t)(&sensor_hub_data.acc)->y * FROM_MG_TO_G;
      sensor_hub_data.acceleration[2] = (float_t)(&sensor_hub_data.acc)->z * FROM_MG_TO_G;
    }

    /* Gyroscope Data */
    if (sensor_hub_data.angular_rate_enable == 1U)
    {
      BSP_SENSOR_GYR_GetAxes(&sensor_hub_data.gyr);
      sensor_hub_data.angular_rate[0] = (float_t)(&sensor_hub_data.gyr)->x * FROM_MDPS_TO_DPS;
      sensor_hub_data.angular_rate[1] = (float_t)(&sensor_hub_data.gyr)->y * FROM_MDPS_TO_DPS;
      sensor_hub_data.angular_rate[2] = (float_t)(&sensor_hub_data.gyr)->z * FROM_MDPS_TO_DPS;
    }

    /* Magnetometer Data */
    if (sensor_hub_data.magnetic_field_enable == 1U)
    {
      BSP_SENSOR_MAG_GetAxes(&sensor_hub_data.mag);
      sensor_hub_data.magnetic_field[0] = (float_t)(&sensor_hub_data.mag)->x * FROM_MGAUSS_TO_UT;
      sensor_hub_data.magnetic_field[1] = (float_t)(&sensor_hub_data.mag)->y * FROM_MGAUSS_TO_UT;
      sensor_hub_data.magnetic_field[2] = (float_t)(&sensor_hub_data.mag)->z * FROM_MGAUSS_TO_UT;
    }

    /* Pressure Sensor Data */
    if (sensor_hub_data.pressure_enable == 1U)
    {
      BSP_SENSOR_PRESS_GetValue(&sensor_hub_data.pressure);
    }

    /* Humidity Sensor Data */
    if (sensor_hub_data.humidity_enable == 1U)
    {
      BSP_SENSOR_HUM_GetValue(&sensor_hub_data.humidity);
    }

    /* Temperature Sensor Data */
    if (sensor_hub_data.temperature_enable == 1U)
    {
      BSP_SENSOR_TEMP_GetValue(&sensor_hub_data.temperature);
    }
  }
  /* FSM, MLC */
  int32_t i;

  if (sensor_hub_data.fsm_enable == 1U)
  {
    for (i = 0; i < 16; i++)
    {
      sensor_hub_data.fsm_data[i] = 0;
    }
  }

  if (sensor_hub_data.mlc_enable == 1U)
  {
    for (i = 0; i < 8; i++)
    {
      sensor_hub_data.mlc_data[i] = 0;
    }
  }

  if (sensor_hub_data.fsm_enable == 1U)
  {
    BSP_ACC_GYR_Read_FSM_Data(sensor_hub_data.fsm_data);
  }

  if (sensor_hub_data.mlc_enable == 1U)
  {
    BSP_ACC_GYR_Read_MLC_Data(sensor_hub_data.mlc_data);
  }

  /* Execute algorithms at 16Hz */
  if (Update16Hz == 1U)
  {
    Update16Hz = 0;
    Algo_16Hz_Handler();
  }

  /* Execute algorithms at 25Hz */
  if (Update25Hz == 1U)
  {
    Update25Hz = 0;
    Algo_25Hz_Handler();
  }

  /* Execute algorithms at 50Hz */
  if (Update50Hz == 1U)
  {
    Update50Hz = 0;
    Algo_50Hz_Handler();
  }

  /* Execute algorithms at 100Hz */
  if (Update100Hz == 1U)
  {
    Update100Hz = 0;
    Algo_100Hz_Handler();
  }

  sensor_hub_data.iteration_counter = sensor_hub_data.iteration_counter + 1U;
}

/**
  * @brief
  */
void Accelero_Init(void)
{
  sensor_hub_data.acceleration_enable = 1;
}

/**
  * @brief
  */
void Gyro_Init(void)
{
  sensor_hub_data.angular_rate_enable = 1;
}

/**
  * @brief
  */
void Magneto_Init(void)
{
  sensor_hub_data.magnetic_field_enable = 1;
}

/**
  * @brief
  */
void Pressure_Init(void)
{
  sensor_hub_data.pressure_enable = 1;
}

/**
  * @brief
  */
void Temperature_Init(void)
{
  sensor_hub_data.temperature_enable = 1;
}

/**
  * @brief
  */
void Humidity_Init(void)
{
  sensor_hub_data.humidity_enable = 1;
}

/**
  * @brief  Handler for algorithms which should be called at 16Hz
  */
void Algo_16Hz_Handler(void)
{
#if (MotionAW)
  if (sensor_hub_data.motion_aw_enable == 1)
  {
    MotionAW_Update_Data();
  }
#endif /* MotionAW */

#if (MotionID)
  if (sensor_hub_data.motion_id_enable == 1)
  {
    MotionID_Update_Data();
  }
#endif /* MotionID */
}

/**
  * @brief  Handler for algorithms which should be called at 25Hz
  */
void Algo_25Hz_Handler(void)
{
#if (MotionTL)
  if (sensor_hub_data.motion_tl_enable == 1)
  {
    MotionTL_Update_Data();
  }
#endif /* MotionTL */
#if (MotionMC)
  if (sensor_hub_data.motion_mc_enable == 1)
  {
    MotionMC_Update_Data();
  }
#endif /* MotionMC */
}

/**
  * @brief  Handler for algorithms which should be called at 50Hz
  */
void Algo_50Hz_Handler(void)
{
#if (MotionAC)
  if (sensor_hub_data.motion_ac_enable == 1)
  {
    MotionAC_Update_Data();
  }
#endif /* MotionAC */
#if (MotionGC)
  if (sensor_hub_data.motion_gc_enable == 1)
  {
    MotionGC_Update_Data();
  }
#endif /* MotionGC */
#if (MotionPM)
  if (sensor_hub_data.motion_pm_enable == 1)
  {
    MotionPM_Update_Data();
  }
#endif /* MotionPM */
#if (MotionPW)
  if (sensor_hub_data.motion_pw_enable == 1)
  {
    MotionPW_Update_Data();
  }
#endif /* MotionPW */
}

/**
  * @brief  Handler for algorithms which should be called at 100Hz
  */
void Algo_100Hz_Handler(void)
{
#if (MotionEC)
  if (sensor_hub_data.motion_ec_enable == 1)
  {
    MotionEC_Update_Data();
  }
#endif /* MotionEC */
#if (MotionFX_9X)
  if (sensor_hub_data.motion_fx_9x_enable == 1)
  {
    MotionFX_9X_Update_Data();
  }
#endif /* MotionFX_9X */
#if (MotionFX_6X)
  if (sensor_hub_data.motion_fx_6x_enable == 1)
  {
    MotionFX_6X_Update_Data();
  }
#endif /* MotionFX_6X */
}

#if (MotionAC)
/**
  * @brief  Accelerometer calibration initialization
  */
void MotionAC_Init(uint32_t mode, float_t threshold)
{
  if (sensor_hub_data.motion_ac_enable == 0)
  {
    MAC_knobs_t knobs;

    MotionAC_Initialize(1);

    MotionAC_GetKnobs(&knobs);

    knobs.Run6PointCal = mode;
    knobs.Sample_ms = 20;
    knobs.MoveThresh_g = threshold;

    MotionAC_SetKnobs(&knobs);

    sensor_hub_data.motion_ac_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
  }
}

/**
  * @brief  Accelerometer calibration data update
  */
void MotionAC_Update_Data(void)
{
  MAC_input_t data_in;
  uint8_t is_calibrated;

  data_in.Acc[0] = sensor_hub_data.acceleration[0];
  data_in.Acc[1] = sensor_hub_data.acceleration[1];
  data_in.Acc[2] = sensor_hub_data.acceleration[2];

  MotionAC_Update(&data_in, &is_calibrated);
  MotionAC_GetCalParams(&sensor_hub_data.mac);
}
#endif /* MotionAC */

#if (MotionAW)
/**
  * @brief  Activity recognition for wrist algorithm initialization
  */
void MotionAW_Init(void)
{
  if (sensor_hub_data.motion_aw_enable == 0)
  {
    char acc_orientation[4];

    MotionAW_Initialize();

    BSP_SENSOR_ACC_GetOrientation(acc_orientation);

    MotionAW_SetOrientation_Acc(acc_orientation);

    sensor_hub_data.motion_aw_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
  }
}

/**
  * @brief  Activity recognition for wrist data update
  */
void MotionAW_Update_Data(void)
{
  MAW_input_t data_in;
  static int64_t time_stamp;

  data_in.AccX = sensor_hub_data.acceleration[0];
  data_in.AccY = sensor_hub_data.acceleration[1];
  data_in.AccZ = sensor_hub_data.acceleration[2];

  MotionAW_Update(&data_in, &sensor_hub_data.maw, time_stamp);
  time_stamp = time_stamp + 63;
}
#endif /* MotionAW */

#if (MotionEC)
/**
  * @brief  ECompass algorithm initialization
  */
void MotionEC_Init(void)
{
  if (sensor_hub_data.motion_ec_enable == 0)
  {
    float_t freq = 100.0f;
    char acc_orientation[4];
    char mag_orientation[4];

    MotionEC_Initialize(MEC_MCU_STM32, &freq);
    MotionEC_SetOrientationEnable(MEC_ENABLE);
    MotionEC_SetVirtualGyroEnable(MEC_ENABLE);
    MotionEC_SetGravityEnable(MEC_ENABLE);
    MotionEC_SetLinearAccEnable(MEC_ENABLE);

    BSP_SENSOR_ACC_GetOrientation(acc_orientation);
    BSP_SENSOR_MAG_GetOrientation(mag_orientation);

    Create_Rotation_Matrix(acc_orientation, sensor_hub_data.acc_matrix);
    Create_Rotation_Matrix(mag_orientation, sensor_hub_data.mag_matrix);

    sensor_hub_data.motion_ec_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
    sensor_hub_data.magnetic_field_enable = 1;

    MotionMC_Init();
  }
}

/**
  * @brief  ECompass algorithm data update
  */
void MotionEC_Update_Data(void)
{
  MEC_input_t mec_data_in;
  float_t cal_mag_data[3];

  Axis_Transformation(mec_data_in.acc, sensor_hub_data.acceleration, sensor_hub_data.acc_matrix);

  cal_mag_data[0] = sensor_hub_data.magnetic_field[0] - sensor_hub_data.mmc.HI_Bias[0];
  cal_mag_data[1] = sensor_hub_data.magnetic_field[1] - sensor_hub_data.mmc.HI_Bias[1];
  cal_mag_data[2] = sensor_hub_data.magnetic_field[2] - sensor_hub_data.mmc.HI_Bias[2];

  Axis_Transformation(mec_data_in.mag, cal_mag_data, sensor_hub_data.mag_matrix);

  mec_data_in.deltatime_s = 10.0f;

  MotionEC_Run(&mec_data_in, &sensor_hub_data.mec);
}
#endif /* MotionEC */

#if (MotionFX_9X)
/**
  * @brief  Sensor fusion algorithm initialization
  */
void MotionFX_9X_Init(void)
{
  if (sensor_hub_data.motion_fx_9x_enable == 0)
  {
    MFX_knobs_t iKnobs;
    MFX_knobs_t *ipKnobs = &iKnobs;

    char acc_orientation[4];
    char gyr_orientation[4];
    char mag_orientation[4];

    MotionFX_initialize((MFXState_t *)mfxstate_9x);

    MotionFX_getKnobs(mfxstate_9x, ipKnobs);

    ipKnobs->gbias_acc_th_sc = GBIAS_ACC_TH_SC_9X;
    ipKnobs->gbias_gyro_th_sc = GBIAS_GYRO_TH_SC_9X;
    ipKnobs->gbias_mag_th_sc = GBIAS_MAG_TH_SC_9X;

    BSP_SENSOR_ACC_GetOrientation(acc_orientation);
    ipKnobs->acc_orientation[0] = acc_orientation[0];
    ipKnobs->acc_orientation[1] = acc_orientation[1];
    ipKnobs->acc_orientation[2] = acc_orientation[2];

    BSP_SENSOR_GYR_GetOrientation(gyr_orientation);
    ipKnobs->gyro_orientation[0] = gyr_orientation[0];
    ipKnobs->gyro_orientation[1] = gyr_orientation[1];
    ipKnobs->gyro_orientation[2] = gyr_orientation[2];

    BSP_SENSOR_MAG_GetOrientation(mag_orientation);
    ipKnobs->mag_orientation[0] = mag_orientation[0];
    ipKnobs->mag_orientation[1] = mag_orientation[1];
    ipKnobs->mag_orientation[2] = mag_orientation[2];

    ipKnobs->output_type = MFX_ENGINE_OUTPUT_ENU;
    ipKnobs->LMode = 1;

#if (defined (ARM_MATH_CM4))
    ipKnobs->modx = 1;
#else
    ipKnobs->modx = 4;
#endif /* ARM_MATH_CM4 */

    MotionFX_setKnobs(mfxstate_9x, ipKnobs);

    MotionFX_enable_6X(mfxstate_9x, MFX_ENGINE_DISABLE);
    MotionFX_enable_9X(mfxstate_9x, MFX_ENGINE_ENABLE);

    MotionFX_MagCal_init(10, 1);

    sensor_hub_data.motion_fx_9x_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
    sensor_hub_data.angular_rate_enable = 1;
    sensor_hub_data.magnetic_field_enable = 1;
  }
}

/**
  * @brief  Sensor fusion algorithm data update
  */
void MotionFX_9X_Update_Data(void)
{
  static uint32_t time_stamp_index;

  /* Magnetometer Calibration */
  MFX_MagCal_input_t mag_data_in;

  mag_data_in.mag[0] = sensor_hub_data.magnetic_field[0] * FROM_UT_TO_UT50;
  mag_data_in.mag[1] = sensor_hub_data.magnetic_field[1] * FROM_UT_TO_UT50;
  mag_data_in.mag[2] = sensor_hub_data.magnetic_field[2] * FROM_UT_TO_UT50;
  mag_data_in.time_stamp = (int32_t)(time_stamp_index * 10);
  time_stamp_index++;

  MotionFX_MagCal_run(&mag_data_in);
  MotionFX_MagCal_getParams(&sensor_hub_data.mfx_magcal);

  /* Sensor Fusion */
  MFX_input_t mfx_data_in;

  mfx_data_in.acc[0] = sensor_hub_data.acceleration[0];
  mfx_data_in.acc[1] = sensor_hub_data.acceleration[1];
  mfx_data_in.acc[2] = sensor_hub_data.acceleration[2];

  mfx_data_in.gyro[0] = sensor_hub_data.angular_rate[0];
  mfx_data_in.gyro[1] = sensor_hub_data.angular_rate[1];
  mfx_data_in.gyro[2] = sensor_hub_data.angular_rate[2];

  mfx_data_in.mag[0] = (sensor_hub_data.magnetic_field[0] - (sensor_hub_data.mfx_magcal.hi_bias[0] * FROM_UT50_TO_UT))
                       * FROM_UT_TO_UT50;
  mfx_data_in.mag[1] = (sensor_hub_data.magnetic_field[1] - (sensor_hub_data.mfx_magcal.hi_bias[1] * FROM_UT50_TO_UT))
                       * FROM_UT_TO_UT50;
  mfx_data_in.mag[2] = (sensor_hub_data.magnetic_field[2] - (sensor_hub_data.mfx_magcal.hi_bias[2] * FROM_UT50_TO_UT))
                       * FROM_UT_TO_UT50;

  float_t delta_time = 0.01f;

  MotionFX_propagate(mfxstate_9x, &sensor_hub_data.mfx_9x, &mfx_data_in, &delta_time);
  MotionFX_update(mfxstate_9x, &sensor_hub_data.mfx_9x, &mfx_data_in, &delta_time, NULL);
}
#endif /* MotionFX_9X */

#if (MotionFX_6X)
/**
  * @brief  Sensor fusion algorithm initialization
  */
void MotionFX_6X_Init(void)
{
  if (sensor_hub_data.motion_fx_6x_enable == 0)
  {
    MFX_knobs_t iKnobs;
    MFX_knobs_t *ipKnobs = &iKnobs;

    char acc_orientation[4];
    char gyr_orientation[4];
    char mag_orientation[4];

    MotionFX_initialize((MFXState_t *)mfxstate_6x);

    MotionFX_getKnobs(mfxstate_6x, ipKnobs);

    ipKnobs->gbias_acc_th_sc = GBIAS_ACC_TH_SC_9X;
    ipKnobs->gbias_gyro_th_sc = GBIAS_GYRO_TH_SC_9X;
    ipKnobs->gbias_mag_th_sc = GBIAS_MAG_TH_SC_9X;

    BSP_SENSOR_ACC_GetOrientation(acc_orientation);
    ipKnobs->acc_orientation[0] = acc_orientation[0];
    ipKnobs->acc_orientation[1] = acc_orientation[1];
    ipKnobs->acc_orientation[2] = acc_orientation[2];

    BSP_SENSOR_GYR_GetOrientation(gyr_orientation);
    ipKnobs->gyro_orientation[0] = gyr_orientation[0];
    ipKnobs->gyro_orientation[1] = gyr_orientation[1];
    ipKnobs->gyro_orientation[2] = gyr_orientation[2];

    BSP_SENSOR_MAG_GetOrientation(mag_orientation);
    ipKnobs->mag_orientation[0] = mag_orientation[0];
    ipKnobs->mag_orientation[1] = mag_orientation[1];
    ipKnobs->mag_orientation[2] = mag_orientation[2];

    ipKnobs->output_type = MFX_ENGINE_OUTPUT_ENU;
    ipKnobs->LMode = 1;

#if (defined (ARM_MATH_CM4))
    ipKnobs->modx = 1;
#else
    ipKnobs->modx = 4;
#endif /* ARM_MATH_CM4 */

    MotionFX_setKnobs(mfxstate_6x, ipKnobs);

    MotionFX_enable_6X(mfxstate_6x, MFX_ENGINE_ENABLE);
    MotionFX_enable_9X(mfxstate_6x, MFX_ENGINE_DISABLE);

    sensor_hub_data.motion_fx_6x_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
    sensor_hub_data.angular_rate_enable = 1;
  }
}

/**
  * @brief  Sensor fusion algorithm data update
  */
void MotionFX_6X_Update_Data(void)
{
  /* Sensor Fusion */
  MFX_input_t mfx_data_in;

  mfx_data_in.acc[0] = sensor_hub_data.acceleration[0];
  mfx_data_in.acc[1] = sensor_hub_data.acceleration[1];
  mfx_data_in.acc[2] = sensor_hub_data.acceleration[2];

  mfx_data_in.gyro[0] = sensor_hub_data.angular_rate[0];
  mfx_data_in.gyro[1] = sensor_hub_data.angular_rate[1];
  mfx_data_in.gyro[2] = sensor_hub_data.angular_rate[2];

  mfx_data_in.mag[0] = 0;
  mfx_data_in.mag[1] = 0;
  mfx_data_in.mag[2] = 0;

  float_t delta_time = 0.01f;

  MotionFX_propagate(mfxstate_6x, &sensor_hub_data.mfx_6x, &mfx_data_in, &delta_time);
  MotionFX_update(mfxstate_6x, &sensor_hub_data.mfx_6x, &mfx_data_in, &delta_time, NULL);
}
#endif /* MotionFX_6X */

#if (MotionGC)
/**
  * @brief  Gyroscope calibration algorithm initialization
  */
void MotionGC_Init(float_t acc_thr, float_t gyro_thr, float_t filter_const, int32_t fast_start, float_t max_acc,
                   float_t max_gyro)
{
  float_t frequency = (float_t)sensor_hub_data.data_rate_Hz;

  if (sensor_hub_data.motion_gc_enable == 0)
  {
    MGC_knobs_t mgc_knobs;

    MotionGC_Initialize(MGC_MCU_STM32, &frequency);
    MotionGC_GetKnobs(&mgc_knobs);
    mgc_knobs.AccThr = acc_thr;
    mgc_knobs.GyroThr = gyro_thr;
    mgc_knobs.FilterConst = filter_const;
    mgc_knobs.FastStart = fast_start;
    mgc_knobs.MaxAcc = max_acc;
    mgc_knobs.MaxGyro = max_gyro;
    MotionGC_SetKnobs(&mgc_knobs);

    sensor_hub_data.motion_gc_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
    sensor_hub_data.angular_rate_enable = 1;
  }
}

/**
  * @brief  Gyroscope calibration algorithm data update
  */
void MotionGC_Update_Data(void)
{
  MGC_input_t mgc_data_in;
  int32_t bias_update;

  mgc_data_in.Acc[0] = sensor_hub_data.acceleration[0];
  mgc_data_in.Acc[1] = sensor_hub_data.acceleration[1];
  mgc_data_in.Acc[2] = sensor_hub_data.acceleration[2];

  mgc_data_in.Gyro[0] = sensor_hub_data.angular_rate[0];
  mgc_data_in.Gyro[1] = sensor_hub_data.angular_rate[1];
  mgc_data_in.Gyro[2] = sensor_hub_data.angular_rate[2];

  MotionGC_Update(&mgc_data_in, &sensor_hub_data.mgc, &bias_update);
}
#endif /* MotionGC */

#if (MotionID)
/**
  * @brief  Motion intensity detection algorithm initialization
  */
void MotionID_Init(void)
{
  if (sensor_hub_data.motion_id_enable == 0)
  {
    MotionID_Initialize(MID_MCU_STM32);

    sensor_hub_data.motion_id_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
  }
}

/**
  * @brief  Motion intensity detection algorithm data update
  */
void MotionID_Update_Data(void)
{
  MID_input_t data_in;

  data_in.AccX = sensor_hub_data.acceleration[0];
  data_in.AccY = sensor_hub_data.acceleration[1];
  data_in.AccZ = sensor_hub_data.acceleration[2];

  MotionID_Update(&data_in, &sensor_hub_data.mid);
}
#endif /* MotionID */

#if (MotionMC)
/**
  * @brief  Magnetometer calibration algorithm initialization
  */
void MotionMC_Init(void)
{
  if (sensor_hub_data.motion_mc_enable == 0)
  {
    MotionMC_Initialize(10, 1);

    sensor_hub_data.motion_mc_enable = 1;
    sensor_hub_data.magnetic_field_enable = 1;
  }
}

/**
  * @brief  Magnetometer calibration algorithm data update
  */
void MotionMC_Update_Data(void)
{
  static uint32_t time_stamp_index;

  MMC_Input_t data_in;

  data_in.Mag[0] = sensor_hub_data.magnetic_field[0];
  data_in.Mag[1] = sensor_hub_data.magnetic_field[1];
  data_in.Mag[2] = sensor_hub_data.magnetic_field[2];
  data_in.TimeStamp = (int32_t)(time_stamp_index * 10);
  time_stamp_index++;

  MotionMC_Update(&data_in);
  MotionMC_GetCalParams(&sensor_hub_data.mmc);
}
#endif /* MotionMC */

#if (MotionPM)
/**
  * @brief  Pedometer for mobile algorithm initialization
  */
void MotionPM_Init(void)
{
  if (sensor_hub_data.motion_pm_enable == 0)
  {
    MotionPM_Initialize();

    sensor_hub_data.motion_pm_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
  }
}

/**
  * @brief  Pedometer for mobile algorithm data update
  */
void MotionPM_Update_Data(void)
{
  MPM_input_t data_in;

  data_in.AccX = sensor_hub_data.acceleration[0];
  data_in.AccY = sensor_hub_data.acceleration[1];
  data_in.AccZ = sensor_hub_data.acceleration[2];

  MotionPM_Update(&data_in, &sensor_hub_data.mpm);
}
#endif /* MotionPM */

#if (MotionPW)
/**
  * @brief  Pedometer for wrist algorithm initialization
  */
void MotionPW_Init(int32_t activity_recognition)
{
  if (sensor_hub_data.motion_pw_enable == 0)
  {
    MotionPW_Initialize();
    MotionPW_ResetStepCount();
    MotionPW_ResetPedometerLibrary();

    sensor_hub_data.motion_pw_enable = 1;
    sensor_hub_data.acceleration_enable = 1;

    if (activity_recognition == 1)
    {
      sensor_hub_data.activity_recognition_enable = 1;
      MotionAW_Init();
    }
    else
    {
      sensor_hub_data.activity_recognition_enable = 0;
    }
  }
}

/**
  * @brief  Pedometer for wrist algorithm data update
  */
void MotionPW_Update_Data(void)
{
  MPW_input_t data_in;
  int32_t activity_conversion = 0;

  data_in.AccX = sensor_hub_data.acceleration[0];
  data_in.AccY = sensor_hub_data.acceleration[1];
  data_in.AccZ = sensor_hub_data.acceleration[2];

  if (sensor_hub_data.activity_recognition_enable == 1)
  {
    /* Convert current activity number */
    activity_conversion = (int32_t)sensor_hub_data.maw.current_activity - 4;

    if ((activity_conversion < (int32_t)MPW_UNKNOWN_ACTIVITY) || (activity_conversion > (int32_t)MPW_JOGGING))
    {
      activity_conversion = (int32_t)MPW_UNKNOWN_ACTIVITY;
    }

    data_in.CurrentActivity = (MPW_activity_t)activity_conversion;
  }
  else
  {
    data_in.CurrentActivity = MPW_WALKING;
  }

  MotionPW_Update(&data_in, &sensor_hub_data.mpw);
}
#endif /* MotionPW */

#if (MotionTL)
/**
  * @brief  Tilt sensing algorithm initialization
  */
void MotionTL_Init(int32_t mode)
{
  if (sensor_hub_data.motion_tl_enable == 0)
  {
    char acc_orientation[3];

    BSP_SENSOR_ACC_GetOrientation(acc_orientation);

    MotionTL_Initialize(MTL_MCU_STM32, acc_orientation);
    MotionTL_SetOrientation_Acc(acc_orientation);

    if (mode == 0)
    {
      sensor_hub_data.mtl_mode = MODE_PITCH_ROLL_GRAVITY_INCLINATION;
    }
    else if (mode == 1)
    {
      sensor_hub_data.mtl_mode = MODE_THETA_PSI_PHI;
    }
    else
    {
      sensor_hub_data.mtl_mode = MODE_THETA_PSI_PHI;
    }

    sensor_hub_data.motion_tl_enable = 1;
    sensor_hub_data.acceleration_enable = 1;
  }
}

/**
  * @brief  Tilt sensing algorithm data update
  */
void MotionTL_Update_Data(void)
{
  MTL_input_t data_in;
  static uint64_t timestamp = 0;

  data_in.acc_x = sensor_hub_data.acceleration[0];
  data_in.acc_y = sensor_hub_data.acceleration[1];
  data_in.acc_z = sensor_hub_data.acceleration[2];
  timestamp = timestamp + 40;

  MotionTL_Update(&data_in, timestamp, &sensor_hub_data.mtl);
}
#endif /* MotionTL */

/**
  * @brief  Get acceleration value
  */
void Accelero_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->acceleration[0];
  data[1] = psensor_hub_data->acceleration[1];
  data[2] = psensor_hub_data->acceleration[2];
}

/**
  * @brief  Get angular rate value
  */
void Gyro_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->angular_rate[0];
  data[1] = psensor_hub_data->angular_rate[1];
  data[2] = psensor_hub_data->angular_rate[2];
}

/**
  * @brief  Get magnetic field value
  */

void Magneto_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->magnetic_field[0];
  data[1] = psensor_hub_data->magnetic_field[1];
  data[2] = psensor_hub_data->magnetic_field[2];
}

/**
  * @brief  Get pressure value
  */

void Pressure_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  *data = psensor_hub_data->pressure;
}

/**
  * @brief  Get temperature value
  */

void Temperature_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  *data = psensor_hub_data->temperature;
}

/**
  * @brief  Get humidity value
  */

void Humidity_Sensor_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  *data = psensor_hub_data->humidity;
}

#if (MotionAC)
/**
  * @brief  Get calibrated acceleration from accelerometer calibration library
  */
void AccCal_GetData(void *pdata, int32_t *reset, float_t *data1, int32_t *data2)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;

  data1[0] = (psensor_hub_data->acceleration[0] - psensor_hub_data->mac.AccBias[0])
             * psensor_hub_data->mac.SF_Matrix[0][0];
  data1[1] = (psensor_hub_data->acceleration[1] - psensor_hub_data->mac.AccBias[1])
             * psensor_hub_data->mac.SF_Matrix[1][1];
  data1[2] = (psensor_hub_data->acceleration[2] - psensor_hub_data->mac.AccBias[2])
             * psensor_hub_data->mac.SF_Matrix[2][2];

  data2[0] = (int32_t)(psensor_hub_data->mac.CalQuality);

  if (*reset == 1)
  {
    MAC_knobs_t mac_knobs;
    MotionAC_GetKnobs(&mac_knobs);
    MotionAC_Initialize(0);
    MotionAC_Initialize(1);
    MotionAC_SetKnobs(&mac_knobs);
  }
}
#endif /* MotionAC */

#if (MotionAW)
/**
  * @brief  Get activity index from activity recognition for wrist library
  */
void ActivityWrist_GetData(void *pdata, int32_t *reset, int32_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = (int32_t)(psensor_hub_data->maw.current_activity);

  if (*reset == 1)
  {
    MotionAW_Reset();
  }
}
#endif /* MotionAW */

#if (MotionEC)
/**
  * @brief  Get quaternions from ecompass library
  */
void MotionEC_Quternions_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mec.quaternion[0];
  data[1] = psensor_hub_data->mec.quaternion[1];
  data[2] = psensor_hub_data->mec.quaternion[2];
  data[3] = psensor_hub_data->mec.quaternion[3];
}

/**
  * @brief  Get rotation vector from ecompass library
  */
void MotionEC_Rotation_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mec.euler[0];
  data[1] = psensor_hub_data->mec.euler[1];
  data[2] = psensor_hub_data->mec.euler[2];
}

/**
  * @brief  Get gravity vector from ecompass library
  */
void MotionEC_Gravity_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mec.gravity[0];
  data[1] = psensor_hub_data->mec.gravity[1];
  data[2] = psensor_hub_data->mec.gravity[2];
}

/**
  * @brief  Get linear acceleration from ecompass library
  */
void MotionEC_LinearAcceleration_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mec.linear[0];
  data[1] = psensor_hub_data->mec.linear[1];
  data[2] = psensor_hub_data->mec.linear[2];
}

/**
  * @brief  Get virtual gyroscope from ecompass library
  */
void MotionEC_VirtualGyroscope_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mec.i_gyro[0];
  data[1] = psensor_hub_data->mec.i_gyro[1];
  data[2] = psensor_hub_data->mec.i_gyro[2];
}
#endif /* MotionEC */

#if (MotionFX_9X)
/**
  * @brief  Get quaternions from sensor fusion library
  */
void Quaternions9X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_9x.quaternion[0];
  data[1] = psensor_hub_data->mfx_9x.quaternion[1];
  data[2] = psensor_hub_data->mfx_9x.quaternion[2];
  data[3] = psensor_hub_data->mfx_9x.quaternion[3];
}

/**
  * @brief  Get rotation vector from sensor fusion library
  */
void Rotation9X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_9x.rotation[0];
  data[1] = psensor_hub_data->mfx_9x.rotation[1];
  data[2] = psensor_hub_data->mfx_9x.rotation[2];
}

/**
  * @brief  Get gravity vector from sensor fusion library
  */
void Gravity9X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_9x.gravity[0];
  data[1] = psensor_hub_data->mfx_9x.gravity[1];
  data[2] = psensor_hub_data->mfx_9x.gravity[2];
}

/**
  * @brief  Get linear acceleration from sensor fusion library
  */
void LinearAcceleration9X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_9x.linear_acceleration[0];
  data[1] = psensor_hub_data->mfx_9x.linear_acceleration[1];
  data[2] = psensor_hub_data->mfx_9x.linear_acceleration[2];
}

/**
  * @brief  Get heading from sensor fusion library
  */
void Heading9X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  *data = psensor_hub_data->mfx_9x.heading;
}

/**
  * @brief  Get calibrated magnetometer from sensor fusion library
  */
void MagnetoCal_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->magnetic_field[0] - (psensor_hub_data->mfx_magcal.hi_bias[0] * FROM_UT50_TO_UT);
  data[1] = psensor_hub_data->magnetic_field[1] - (psensor_hub_data->mfx_magcal.hi_bias[1] * FROM_UT50_TO_UT);
  data[2] = psensor_hub_data->magnetic_field[2] - (psensor_hub_data->mfx_magcal.hi_bias[2] * FROM_UT50_TO_UT);
}

/**
  * @brief  Get calibration parameters from sensor fusion library
  */
void MagnetoCal_GetCalData(void *pdata, float_t *data, int32_t *quality)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_magcal.hi_bias[0] * FROM_UT50_TO_UT;
  data[1] = psensor_hub_data->mfx_magcal.hi_bias[1] * FROM_UT50_TO_UT;
  data[2] = psensor_hub_data->mfx_magcal.hi_bias[2] * FROM_UT50_TO_UT;

  *quality = (int32_t)(psensor_hub_data->mfx_magcal.cal_quality);
}
#endif /* MotionFX */

#if (MotionFX_6X)
/**
  * @brief  Get quaternions from sensor fusion library
  */
void Quaternions6X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_6x.quaternion[0];
  data[1] = psensor_hub_data->mfx_6x.quaternion[1];
  data[2] = psensor_hub_data->mfx_6x.quaternion[2];
  data[3] = psensor_hub_data->mfx_6x.quaternion[3];
}

/**
  * @brief  Get rotation vector from sensor fusion library
  */
void Rotation6X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_6x.rotation[0];
  data[1] = psensor_hub_data->mfx_6x.rotation[1];
  data[2] = psensor_hub_data->mfx_6x.rotation[2];
}

/**
  * @brief  Get gravity vector from sensor fusion library
  */
void Gravity6X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_6x.gravity[0];
  data[1] = psensor_hub_data->mfx_6x.gravity[1];
  data[2] = psensor_hub_data->mfx_6x.gravity[2];
}

/**
  * @brief  Get linear acceleration from sensor fusion library
  */
void LinearAcceleration6X_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->mfx_6x.linear_acceleration[0];
  data[1] = psensor_hub_data->mfx_6x.linear_acceleration[1];
  data[2] = psensor_hub_data->mfx_6x.linear_acceleration[2];
}
#endif /* MotionFX_6X */

#if (MotionGC)
/**
  * @brief  Get calibrated angular rate from gyroscope calibration library
  */
void GyroCal_GetData(void *pdata, int32_t *reset, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = psensor_hub_data->angular_rate[0] - psensor_hub_data->mgc.GyroBiasX;
  data[1] = psensor_hub_data->angular_rate[1] - psensor_hub_data->mgc.GyroBiasY;
  data[2] = psensor_hub_data->angular_rate[2] - psensor_hub_data->mgc.GyroBiasZ;

  float_t frequency = (float_t)sensor_hub_data.data_rate_Hz;

  if (*reset == 1)
  {
    MGC_knobs_t mgc_knobs;
    MotionGC_GetKnobs(&mgc_knobs);
    MotionGC_Initialize(MGC_MCU_STM32, &frequency);
    MotionGC_SetKnobs(&mgc_knobs);
  }
}
#endif /* MotionGC */

#if (MotionID)
/**
  * @brief  Get motion intensity index from motion intensity detection library
  */
void MotionIntensity_GetData(void *pdata, int32_t *reset, int32_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data[0] = (int32_t)(psensor_hub_data->mid);

  if (*reset == 1)
  {
    MotionID_ResetLib();
  }
}
#endif /* MotionID */

#if (MotionMC)
/**
  * @brief  Get calibrated magnetic field and calibration quality from magnetometer calibration library
  */
void MagCal_GetData(void *pdata, int32_t *reset, float_t *data1, int32_t *data2)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data1[0] = psensor_hub_data->magnetic_field[0] - psensor_hub_data->mmc.HI_Bias[0];
  data1[1] = psensor_hub_data->magnetic_field[1] - psensor_hub_data->mmc.HI_Bias[1];
  data1[2] = psensor_hub_data->magnetic_field[2] - psensor_hub_data->mmc.HI_Bias[2];

  data2[0] = (int32_t)(psensor_hub_data->mmc.CalQuality);

  if (*reset == 1)
  {
    MotionMC_Initialize(10, 0);
    MotionMC_Initialize(10, 1);
  }
}
#endif /* MotionMC */

#if (MotionPM)
/**
  * @brief  Get number of steps and cadency from pedometer for mobile library
  */
void PedometerMobile_GetData(void *pdata, int32_t *reset, int32_t *data1, int32_t *data2)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data1[0] = (int32_t)(psensor_hub_data->mpm.Nsteps);
  data2[0] = (int32_t)(psensor_hub_data->mpm.Cadence);

  if (*reset == 1)
  {
    MotionPM_Initialize();
  }
}
#endif /* MotionPM */

#if (MotionPW)
/**
  * @brief  Get number of steps and cadency from pedometer for wrist library
  */
void PedometerWrist_GetData(void *pdata, int32_t *reset, int32_t *data1, int32_t *data2)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  data1[0] = (int32_t)(psensor_hub_data->mpw.Nsteps);
  data2[0] = (int32_t)(psensor_hub_data->mpw.Cadence);

  if (*reset == 1)
  {
    MotionPW_ResetStepCount();
    MotionPW_ResetPedometerLibrary();
  }
}
#endif /* MotionPW */

#if (MotionTL)
/**
  * @brief  Get tilt angles from tilt sensing library
  */
void TiltSensing_GetData(void *pdata, float_t *data)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;

  if (psensor_hub_data->mtl_mode == MODE_PITCH_ROLL_GRAVITY_INCLINATION)
  {
    data[0] = psensor_hub_data->mtl.pitch_3x;
    data[1] = psensor_hub_data->mtl.roll_3x;
    data[2] = psensor_hub_data->mtl.phi_3x;
  }
  else if (psensor_hub_data->mtl_mode == MODE_THETA_PSI_PHI)
  {
    data[0] = psensor_hub_data->mtl.theta_3x;
    data[1] = psensor_hub_data->mtl.psi_3x;
    data[2] = psensor_hub_data->mtl.phi_3x;
  }
  else
  {
    data[0] = psensor_hub_data->mtl.theta_3x;
    data[1] = psensor_hub_data->mtl.psi_3x;
    data[2] = psensor_hub_data->mtl.phi_3x;
  }
}
#endif /* MotionTL */

/**
  * @brief  Convert microtesla to miligauss
  */
void TeslaToGauss(float_t *in, float_t *out)
{
  out[0] = in[0] * FROM_UT_TO_MGAUSS;
  out[1] = in[1] * FROM_UT_TO_MGAUSS;
  out[2] = in[2] * FROM_UT_TO_MGAUSS;
}

/**
  * @brief  Return iteration counter value
  */
void GetIterationCounter(void *pdata, int32_t *out)
{
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;
  out[0] = (int32_t) psensor_hub_data->iteration_counter;
}

/**
  * @brief  FSM, MLC init
  */
void FSM_MLC_Init(int32_t fsm_number, int32_t mlc_number)
{
  uint32_t i;
  uint32_t length;

  acc_odr_before_ucf = Get_Accelero_ODR();
  acc_fs_before_ucf = Get_Accelero_FS();
  gyro_odr_before_ucf = Get_Gyro_ODR();
  gyro_fs_before_ucf = Get_Gyro_FS();

  if ((fsm_number > 0) || (mlc_number > 0))
  {
    length = sizeof(fsm_mlc_config) / sizeof(ucf_line_t);
    for (i = 0; i < length; i++)
    {
      (void)BSP_SENSOR_ACC_Write_Register(fsm_mlc_config[i].address, fsm_mlc_config[i].data);
    }

    if (fsm_number > 0)
    {
      sensor_hub_data.fsm_enable = 1;
    }

    if (mlc_number > 0)
    {
      sensor_hub_data.mlc_enable = 1;
    }
  }
}

/**
  * @brief  FSM, MLC Check if ucf file loading changes initial value of ODR or FS
  */
void FSM_MLC_Check_ODR_FS_Change()
{
  if ((sensor_hub_data.fsm_enable != 0U) || (sensor_hub_data.mlc_enable != 0U))
  {
    uint32_t gyro_fs = Get_Gyro_FS();
    float_t gyro_odr = Get_Gyro_ODR();
    uint32_t acc_fs = Get_Accelero_FS();
    float_t acc_odr = Get_Accelero_ODR();
    uint32_t ODR_changed = 0;
    uint32_t FS_changed = 0;

    if (acc_odr_before_ucf  > acc_odr)
    {
      ODR_changed = 1;
    }
    if (acc_fs_before_ucf  != acc_fs)
    {
      FS_changed = 1;
    }

    if (sensor_hub_data.angular_rate_enable != 0U)
    {
      if (gyro_odr_before_ucf > gyro_odr)
      {
        ODR_changed = 1;
      }
      if (gyro_fs_before_ucf != gyro_fs)
      {
        FS_changed = 1;
      }
    }
    if (ODR_changed != 0U)
    {
      Report_Init_Error(AB_ERROR_ODR_CHANGED);
    }
    if (FS_changed != 0U)
    {
      Report_Init_Error(AB_ERROR_FS_CHANGED);
    }
  }
}

/**
  * @brief  FSM, MLC get data
  */
void FSM_MLC_GetData(void *pdata, int32_t *fsm_data, int32_t *mlc_data, int32_t fsm_number, int32_t mlc_number)
{
  int32_t i;
  sensor_hub_data_t *psensor_hub_data = (sensor_hub_data_t *) pdata;

  for (i = 0; i < fsm_number; i++)
  {
    fsm_data[i] = (int32_t) psensor_hub_data->fsm_data[i];
  }

  for (i = 0; i < mlc_number; i++)
  {
    mlc_data[i] = (int32_t) psensor_hub_data->mlc_data[i];
  }
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
  * @brief  Get accelerometer full scale
  */
static uint32_t Get_Accelero_FS()
{
  int32_t fs = 0;

  BSP_SENSOR_GYR_GetFullScale(&fs);

  return (uint32_t)fs;
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
  * @brief  Get gyroscope full scale
  */
static uint32_t Get_Gyro_FS()
{
  int32_t fs = 0;

  BSP_SENSOR_GYR_GetFullScale(&fs);

  return (uint32_t)fs;
}

/**
  * @brief  Get Accelero output data rate
  */
static float_t Get_Accelero_ODR()
{
  float_t odr = 0.0f;

  BSP_SENSOR_ACC_GetOutputDataRate(&odr);

  return odr;
}

/**
  * @brief  Get Gyro output data rate
  */
static float_t Get_Gyro_ODR()
{
  float_t odr = 0.0f;

  BSP_SENSOR_ACC_GetOutputDataRate(&odr);

  return odr;
}

#if (MotionAC)
/**
  * @brief Load the calibration parameters from storage
  * @param dataSize  size of data
  * @param data  pointer to data
  * @retval Will return 0 the if it is success and 1 if it is failure
 */
char MotionAC_LoadCalFromNVM(uint16_t datasize, uint32_t *data)
{
  return 1; /* Read from NVM not implemented. */
}

/**
  * @brief Save the calibration parameters in storage
  * @param dataSize  size of data
  * @param data  pointer to data
  * @retval Will return 0 the if it is success and 1 if it is failure
 */
char MotionAC_SaveCalInNVM(uint16_t datasize, uint32_t *data)
{
  return 1; /* Write to NVM not implemented. */
}
#endif /* MotionAC */

#if (MotionFX)
/**
  * @brief  Load calibration parameter from memory
  * @param  dataSize length of the data
  * @param  data pointer to the data
  * @retval (1) fail, (0) success
  */
char MotionFX_LoadMagCalFromNVM(uint16_t dataSize, uint32_t *data)
{
  return 1;  /* Write to NVM not implemented. */
}

/**
  * @brief  Save calibration parameter to memory
  * @param  dataSize length of the data
  * @param  data pointer to the data
  * @retval (1) fail, (0) success
  */
char MotionFX_SaveMagCalInNVM(uint16_t dataSize, uint32_t *data)
{
  return 1;  /* Write to NVM not implemented. */
}
#endif /* MotionFX */

#if (MotionMC)
/**
  * @brief Load the calibration parameters from storage
  * @param dataSize  size of data
  * @param data  pointer to data
  * @retval Will return 0 the if it is success and 1 if it is failure
 */
char MotionMC_LoadCalFromNVM(uint16_t datasize, uint32_t *data)
{
  return 1; /* Read from NVM not implemented. */
}

/**
  * @brief Save the calibration parameters in storage
  * @param dataSize  size of data
  * @param data  pointer to data
  * @retval Will return 0 the if it is success and 1 if it is failure
 */
char MotionMC_SaveCalInNVM(uint16_t datasize, uint32_t *data)
{
  return 1; /* Write to NVM not implemented. */
}
#endif /* MotionMC */

#if (MotionEC)
/**
  * @brief  Creates the rotation matrix that allows to change from original reference system to ENU
 */
static void Create_Rotation_Matrix(const char *orientation, float_t *matrix)
{
  uint8_t i;
  char ao[3];

  for (i = 0; i < 9; i++)
  {
    matrix[i] = 0;
  }

  for (i = 0; i < 3; i++)
  {
    if ((orientation[i] > 'A') && (orientation[i] < 'Z'))
    {
      ao[i] = orientation[i] + ('a' - 'A');
    }
    else
    {
      ao[i] = orientation[i];
    }
  }

  switch (ao[0])
  {
    case 'e':
      matrix[0] = 1.0f;
      break;
    case 'n':
      matrix[3] = 1.0f;
      break;
    case 'w':
      matrix[0] = -1.0f;
      break;
    case 's':
      matrix[3] = -1.0f;
      break;
    case 'u':
      matrix[6] = 1.0f;
      break;
    case 'd':
      matrix[6] = -1.0f;
      break;
  }

  switch (ao[1])
  {
    case 'e':
      matrix[1] = 1.0f;
      break;
    case 'n':
      matrix[4] = 1.0f;
      break;
    case 'w':
      matrix[1] = -1.0f;
      break;
    case 's':
      matrix[4] = -1.0f;
      break;
    case 'u':
      matrix[7] = 1.0f;
      break;
    case 'd':
      matrix[7] = -1.0f;
      break;
  }

  switch (ao[2])
  {
    case 'e':
      matrix[2] = 1.0f;
      break;
    case 'n':
      matrix[5] = 1.0f;
      break;
    case 'w':
      matrix[2] = -1.0f;
      break;
    case 's':
      matrix[5] = -1.0f;
      break;
    case 'u':
      matrix[8] = 1.0f;
      break;
    case 'd':
      matrix[8] = -1.0f;
      break;
  }
}

/**
  * @brief  Performs the axis transformation
 */
static void Axis_Transformation(float_t *o, float_t *i, float_t *matrix)
{
  o[0] = matrix[0] * i[0] + matrix[1] * i[1] + matrix[2] * i[2];
  o[1] = matrix[3] * i[0] + matrix[4] * i[1] + matrix[5] * i[2];
  o[2] = matrix[6] * i[0] + matrix[7] * i[1] + matrix[8] * i[2];
}
#endif /* MotionEC */

/**
  * @}
  */

/**
  * @}
  */
