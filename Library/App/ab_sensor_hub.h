/**
  *******************************************************************************
  * @file    ab_sensor_hub.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_sensor_hub.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef AB_SENSOR_HUB_H
#define AB_SENSOR_HUB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ab_libraries.h"
#include "app_algobuild.h"

#if (MotionAC)
#include "motion_ac.h"
#endif /* MotionAC */
#if (MotionAW)
#include "motion_aw.h"
#endif /* MotionAW */
#if (MotionEC)
#include "motion_ec.h"
#endif /* MotionEC */
#if ((MotionFX_6X) || (MotionFX_9X))
#include "motion_fx.h"
#endif /* MotionFX_6X) || (MotionFX_9X */
#if (MotionGC)
#include "motion_gc.h"
#endif /* MotionGC */
#if (MotionID)
#include "motion_id.h"
#endif /* MotionID */
#if (MotionMC)
#include "motion_mc.h"
#endif /* MotionMC */
#if (MotionPM)
#include "motion_pm.h"
#endif /* MotionPM */
#if (MotionPW)
#include "motion_pw.h"
#endif /* MotionPW */
#if (MotionTL)
#include "motion_tl.h"
#endif /* MotionTL */

#include "demo_serial.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  DRC_TIMER = 0,
  DRC_ACCELEROMETER = 1,
  DRC_GYROSCOPE = 2,
  DRC_OFFLINE = 255,
} data_rate_control_t;

typedef struct
{
  data_rate_control_t data_rate_control;
  uint32_t data_rate_Hz;
  uint32_t meas_data_rate_Hz;
  MOTION_SENSOR_Axes_t acc;
  MOTION_SENSOR_Axes_t gyr;
  MOTION_SENSOR_Axes_t mag;
  uint64_t timestamp_us;
  float_t acceleration[3];
  float_t angular_rate[3];
  float_t magnetic_field[3];
  float_t pressure;
  float_t humidity;
  float_t temperature;
#if (MotionAC)
  MAC_output_t mac;
#endif /* MotionAC */
#if (MotionAW)
  MAW_output_t maw;
#endif /* MotionAW */
#if (MotionEC)
  MEC_output_t mec;
  float_t acc_matrix[9];
  float_t mag_matrix[9];
#endif /* MotionEC */
#if (MotionFX_9X)
  MFX_MagCal_output_t mfx_magcal;
  MFX_output_t mfx_9x;
#endif /* MotionFX_9X */
#if (MotionFX_6X)
  MFX_output_t mfx_6x;
#endif /* MotionFX_6X */
#if (MotionGC)
  MGC_output_t mgc;
#endif /* MotionGC */
#if (MotionID)
  MID_output_t mid;
#endif /* MotionID */
#if (MotionMC)
  MMC_Output_t mmc;
#endif /* MotionMC */
#if (MotionPM)
  MPM_output_t mpm;
#endif /* MotionPM */
#if (MotionPW)
  MPW_output_t mpw;
  uint8_t activity_recognition_enable;
#endif /* MotionPW */
#if (MotionTL)
  MTL_output_t mtl;
  MTL_angle_mode_t mtl_mode;
#endif /* MotionTL */
  uint8_t acceleration_enable;
  uint8_t angular_rate_enable;
  uint8_t magnetic_field_enable;
  uint8_t pressure_enable;
  uint8_t humidity_enable;
  uint8_t temperature_enable;
  uint8_t motion_ac_enable;
  uint8_t motion_aw_enable;
  uint8_t motion_ec_enable;
  uint8_t motion_fx_9x_enable;
  uint8_t motion_fx_6x_enable;
  uint8_t motion_gc_enable;
  uint8_t motion_id_enable;
  uint8_t motion_mc_enable;
  uint8_t motion_pm_enable;
  uint8_t motion_pw_enable;
  uint8_t motion_tl_enable;
  uint32_t iteration_counter;
  uint8_t fsm_enable;
  uint8_t fsm_data[16];
  uint8_t mlc_enable;
  uint8_t mlc_data[8];
} sensor_hub_data_t;

/* Exported constants --------------------------------------------------------*/
#define FROM_MG_TO_G          0.001f
#define FROM_G_TO_MG          1000.0f
#define FROM_MDPS_TO_DPS      0.001f
#define FROM_DPS_TO_MDPS      1000.0f
#define FROM_MGAUSS_TO_UT50   (0.1f/50.0f)
#define FROM_MGAUSS_TO_UT     0.1f
#define FROM_UT_TO_UT50       0.02f
#define FROM_UT_TO_MGAUSS     10.0f
#define FROM_UT50_TO_UT       50.0f
#define FROM_UT50_TO_MGAUSS   500.0f

#define SAMPLETODISCARD       15
#define GBIAS_ACC_TH_SC_6X    (2.0f*0.000765f)
#define GBIAS_GYRO_TH_SC_6X   (2.0f*0.002f)
#define GBIAS_MAG_TH_SC_6X    (2.0f*0.001500f)
#define GBIAS_ACC_TH_SC_9X    (2.0f*0.000765f)
#define GBIAS_GYRO_TH_SC_9X   (2.0f*0.002f)
#define GBIAS_MAG_TH_SC_9X    (2.0f*0.001500f)

/* External variables --------------------------------------------------------*/
extern sensor_hub_data_t sensor_hub_data;
extern float_t acc_odr_before_ucf;
extern uint32_t acc_fs_before_ucf;
extern float_t gyro_odr_before_ucf;
extern uint32_t gyro_fs_before_ucf;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Sensor_Hub_Init(uint32_t data_rate_control, uint32_t data_rate, uint32_t accel_fs, uint32_t gyro_fs);
void Sensor_Hub_Handler(void **pdata);
void Sensor_Hub_Offline_Handler(void **pdata);

void Accelero_Init(void);
void Gyro_Init(void);
void Magneto_Init(void);
void Pressure_Init(void);
void Temperature_Init(void);
void Humidity_Init(void);

void Algo_16Hz_Handler(void);
void Algo_25Hz_Handler(void);
void Algo_50Hz_Handler(void);
void Algo_100Hz_Handler(void);

#if (MotionAC)
void MotionAC_Init(uint32_t mode, float_t threshold);
#endif /* MotionAC */
#if (MotionAW)
void MotionAW_Init(void);
#endif /* MotionAW */
#if (MotionEC)
void MotionEC_Init(void);
#endif /* MotionEC */
#if (MotionFX_9X)
void MotionFX_9X_Init(void);
#endif /* MotionFX_9X */
#if (MotionFX_6X)
void MotionFX_6X_Init(void);
#endif /* MotionFX_6X */
#if (MotionGC)
void MotionGC_Init(float_t acc_thr, float_t gyro_thr, float_t filter_const, int32_t fast_start, float_t max_acc,
                   float_t max_gyro);
#endif /* MotionGC */
#if (MotionID)
void MotionID_Init(void);
#endif /* MotionID */
#if (MotionMC)
void MotionMC_Init(void);
#endif /* MotionMC */
#if (MotionPM)
void MotionPM_Init(void);
#endif /* MotionPM */
#if (MotionPW)
void MotionPW_Init(int32_t activity_recognition);
#endif /* MotionPW */
#if (MotionTL)
void MotionTL_Init(int32_t mode);
#endif /* MotionTL */

void Accelero_Sensor_GetData(void *pdata, float_t *data);
void Gyro_Sensor_GetData(void *pdata, float_t *data);
void Magneto_Sensor_GetData(void *pdata, float_t *data);
void Pressure_Sensor_GetData(void *pdata, float_t *data);
void Temperature_Sensor_GetData(void *pdata, float_t *data);
void Humidity_Sensor_GetData(void *pdata, float_t *data);

void Quaternions9X_GetData(void *pdata, float_t *data);
void Rotation9X_GetData(void *pdata, float_t *data);
void Gravity9X_GetData(void *pdata, float_t *data);
void LinearAcceleration9X_GetData(void *pdata, float_t *data);
void Heading9X_GetData(void *pdata, float_t *data);
void MagnetoCal_GetData(void *pdata, float_t *data);
void MagnetoCal_GetCalData(void *pdata, float_t *data, int32_t *quality);

void Quaternions6X_GetData(void *pdata, float_t *data);
void Rotation6X_GetData(void *pdata, float_t *data);
void Gravity6X_GetData(void *pdata, float_t *data);
void LinearAcceleration6X_GetData(void *pdata, float_t *data);

void AccCal_GetData(void *pdata, int32_t *reset, float_t *data1, int32_t *data2);
void ActivityWrist_GetData(void *pdata, int32_t *reset, int32_t *data);
void MotionEC_Quternions_GetData(void *pdata, float_t *data);
void MotionEC_Rotation_GetData(void *pdata, float_t *data);
void MotionEC_Gravity_GetData(void *pdata, float_t *data);
void MotionEC_LinearAcceleration_GetData(void *pdata, float_t *data);
void MotionEC_VirtualGyroscope_GetData(void *pdata, float_t *data);
void GyroCal_GetData(void *pdata, int32_t *reset, float_t *data);
void MotionIntensity_GetData(void *pdata, int32_t *reset, int32_t *data);
void MagCal_GetData(void *pdata, int32_t *reset, float_t *data1, int32_t *data2);
void PedometerMobile_GetData(void *pdata, int32_t *reset, int32_t *data1, int32_t *data2);
void PedometerWrist_GetData(void *pdata, int32_t *reset, int32_t *data1, int32_t *data2);
void TiltSensing_GetData(void *pdata, float_t *data);

void FSM_MLC_Init(int32_t fsm_number, int32_t mlc_number);
void FSM_MLC_GetData(void *pdata, int32_t *fsm_data, int32_t *mlc_data, int32_t fsm_number, int32_t mlc_number);
void FSM_MLC_Check_ODR_FS_Change(void);

void TeslaToGauss(float_t *in, float_t *out);
void GetIterationCounter(void *pdata, int32_t *out);

#ifdef __cplusplus
}
#endif

#endif /* AB_SENSOR_HUB_H */
