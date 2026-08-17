/**
  *******************************************************************************
  * @file    ab_signal.h
  * @author  MEMS Software Solutions Team
  * @brief   Header for ab_signal.c
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
#ifndef AB_SIGNAL_H
#define AB_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ab_math.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define AB_FALLING_EDGE  1 /* negative zero crossing */
#define AB_RISING_EDGE   2 /* positive zero crossing */
#define NEGATIVE_PEAK 1
#define POSITIVE_PEAK 2

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void PulseWidth(int32_t *in, int32_t *out, int32_t edge, int32_t *mem);
void PulseGenerator(int32_t *in, int32_t *out, int32_t time, int32_t *mem);
void MovingAverage(float_t *in, float_t *out, uint32_t length, float_t *mem1, int32_t *mem2);
void Counter(int32_t *in, int32_t *out, int32_t edge, int32_t *mem);

void FIR_Filter_Init(arm_fir_instance_f32 *pfilter, uint16_t numTaps, float32_t *pcoeffs, float32_t *pstate);
void FIR_Filter(arm_fir_instance_f32 *pfilter, float32_t *psrc, float32_t *pdst);
void IIR_Filter_Init(arm_biquad_casd_df1_inst_f32 *pfilter, uint8_t numStages, float32_t *pcoeffs, float32_t *pstate);
void IIR_Filter(arm_biquad_casd_df1_inst_f32 *pfilter, float32_t *psrc, float32_t *pdst);

void Integrator(float_t *in, float_t *out, float_t *mem);
void Derivator(float_t *in, float_t *out, float_t *mem);

void SignalSwitchInt(int32_t *in1, int32_t *in2, int32_t *control, int32_t *out);
void SignalSwitchFloat(float_t *in1, float_t *in2, int32_t *control, float_t *out);

void Feature_Computation(float_t *in, int32_t *reset, float_t *max, float_t *min, float_t *mean, float_t *var,
                         float_t *peak, float_t *energy, uint32_t length, int32_t *mem, float_t *buffer);

void Feature_Computation_2_Init(float_t *buffer, int32_t n_items);
void Feature_Computation_2(float_t *in, int32_t *reset, float_t *max, float_t *min, float_t *mean, float_t *var,
                           float_t *peak, float_t *energy, int32_t *updated, uint32_t length, int32_t *mem,
                           float_t *buffer);

void Signal_Delay(float_t *in, float_t *out, uint32_t length, float_t *mem1, int32_t *mem2);

void Zero_Crossing(float_t *in, int32_t *out, uint32_t edge, float_t *mem);
void Zero_Crossing_Hyst(float_t *in, int32_t *out, uint32_t edge, float_t hyst, int32_t *stat);
void Peak_Detector(float_t *in, int32_t *out, uint32_t peak, float_t threshold, float_t *mem);
void Peak_Detector_RelTh(float_t *in, int32_t *out, uint32_t peak, float_t threshold, int32_t reset, int32_t *stat,
                         float_t *mem);
void Spike_Detector(float_t *in, int32_t *out, uint32_t peak, float_t threshold, float_t *mem);

#ifdef __cplusplus
}
#endif

#endif /* AB_SIGNAL_H */
