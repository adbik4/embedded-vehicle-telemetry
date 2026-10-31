/**
  ******************************************************************************
  * @file    algo_builder.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder functions
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
#include "algo_builder.h"

const char Identification_String[] = "ID_STRING:Example_02.xml,On-line";

static void Mux_3_float(float_t *in1, float_t *in2, float_t *in3, float_t *out)
{
  out[0] = *in1;
  out[1] = *in2;
  out[2] = *in3;
}

sDISPLAY_INFO display_info_list[] =
{
  {INFO_TYPE_GRAPH, 1, 3, VAR_TYPE_FLOAT, 0, "graph|Acceleration|[g]|4|19|X axis|Y axis|Z axis||||0", 0, 0},
  {INFO_TYPE_SCATTER, 2, 3, VAR_TYPE_FLOAT, 12, "scatter|Magnetic\nField|[uT]|1|Value 1|Value 2|Value 3|0", 0, 0},
  {
    INFO_TYPE_FLOAT, 3, 3, VAR_TYPE_FLOAT, 24,
    "float|Output\nValue|Pressure|[hPa]|Temperature|[C]|Humidity|[percent]||||||||||", 0, 0
  },
  {INFO_TYPE_GRAPH, 4, 3, VAR_TYPE_FLOAT, 36, "graph|Angular Rate|[dps]|4|19|X axis|Y axis|Z axis||||0", 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

void AB_Init(void)
{
  Sensor_Hub_Init(0, 100, 1, 1);
  Accelero_Init();
  Gyro_Init();
  Magneto_Init();
  Pressure_Init();
  Temperature_Init();
  Humidity_Init();
  Message_Length = 48;
}

void AB_Handler(void)
{
  float_t Acceleration_g_2_data[3];
  float_t Angular_Rate_dps_1_data[3];
  float_t Humidity_percent_1_data[1];
  float_t Magnetic_Field_uT_1_data[3];
  float_t Mux_Float_1_out[3];
  float_t Pressure_hPa_1_data[1];
  float_t Temperature_C_2_data[1];
  void *Sensor_Hub_1_out;

  Sensor_Hub_Handler(&Sensor_Hub_1_out);
  Accelero_Sensor_GetData(Sensor_Hub_1_out, Acceleration_g_2_data);
  Gyro_Sensor_GetData(Sensor_Hub_1_out, Angular_Rate_dps_1_data);
  Magneto_Sensor_GetData(Sensor_Hub_1_out, Magnetic_Field_uT_1_data);
  Pressure_Sensor_GetData(Sensor_Hub_1_out, Pressure_hPa_1_data);
  Temperature_Sensor_GetData(Sensor_Hub_1_out, Temperature_C_2_data);
  Humidity_Sensor_GetData(Sensor_Hub_1_out, Humidity_percent_1_data);
  Mux_3_float(Pressure_hPa_1_data, Temperature_C_2_data, Humidity_percent_1_data, Mux_Float_1_out);
  Display_Update(Acceleration_g_2_data, &display_info_list[0]);
  Display_Update(Magnetic_Field_uT_1_data, &display_info_list[1]);
  Display_Update(Mux_Float_1_out, &display_info_list[2]);
  Display_Update(Angular_Rate_dps_1_data, &display_info_list[3]);
}
