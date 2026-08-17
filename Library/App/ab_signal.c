/**
  ******************************************************************************
  * @file    ab_signal.c
  * @author  MEMS Software Solutions Team
  * @brief   AlgoBuilder signal processing functions
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
#include "ab_signal.h"
#include "ab_sensor_hub.h"
#include "float.h"

/** @addtogroup AlgoBuilder_Firmware
  * @{
  */

/** @addtogroup AlgoBuilder_Signal_Processing
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Measures pulse width
  */
void PulseWidth(int32_t *in, int32_t *out, int32_t edge, int32_t *mem)
{
  if (mem[0] == 0)
  {
    if (in[0] == edge)
    {
      mem[0] = 1;
      mem[1] = (int32_t)HAL_GetTick();
    }
    *out = 0;
  }
  else if (mem[0] == 1)
  {
    if (in[0] != edge)
    {
      *out = (int32_t)HAL_GetTick() - mem[1];
      mem[0] = 0;
      mem[1] = 0;
    }
    else
    {
      *out = 0;
    }
  }
  else
  {
    /* Do nothing */
  }
}

/**
  * @brief  Generated pulse with selected width
  */
void PulseGenerator(int32_t *in, int32_t *out, int32_t time, int32_t *mem)
{
  int32_t delay;

  if (mem[0] == 0)
  {
    if (in[0] != 0)
    {
      mem[0] = 1;
      mem[1] = (int32_t)HAL_GetTick();
      *out = 1;
    }
  }
  else if (mem[0] == 1)
  {
    delay = (int32_t)HAL_GetTick() - mem[1];

    if (delay >= time)
    {
      *out = 0;
      mem[0] = 0;
      mem[1] = 0;
    }
    else
    {
      *out = 1;
    }
  }
  else
  {
    /* Do nothing */
  }
}

/**
  * @brief  Calculates moving average
  */
void MovingAverage(float_t *in, float_t *out, uint32_t length, float_t *mem1, int32_t *mem2)
{
  float_t sum = 0.0f;

  mem2[0] = (mem2[0] + 1) % (int32_t)length;

  mem1[mem2[0]] = *in;

  for (uint32_t i = 0; i < length; i++)
  {
    sum += mem1[i];
  }

  if (length != 0U)
  {
    *out = sum / (float_t)length;
  }
  else
  {
    *out = 0.0f;
  }
}

/**
  * @brief  Counts number of pulses
  */
void Counter(int32_t *in, int32_t *out, int32_t edge, int32_t *mem)
{
  if (mem[0] == 0)
  {
    if (in[0] != edge)
    {
      mem[0] = 1;
      mem[1] += 1;
    }
  }
  else if (mem[0] == 1)
  {
    if (in[0] == edge)
    {
      mem[0] = 0;
    }
  }
  else
  {
    /* Nothing to do */
  }

  if (edge == 1)
  {
    *out = (mem[1] - 1);
  }
  else
  {
    *out = mem[1];
  }
}

/**
  * @brief  Initializes FIR filter
  */
void FIR_Filter_Init(arm_fir_instance_f32 *pfilter, uint16_t numTaps, float32_t *pcoeffs, float32_t *pstate)
{
  arm_fir_init_f32(pfilter, numTaps, pcoeffs, pstate, 1);
}

/**
  * @brief  Update FIR filter output value
  */
void FIR_Filter(arm_fir_instance_f32 *pfilter, float32_t *psrc, float32_t *pdst)
{
  arm_fir_f32(pfilter, psrc, pdst, 1);
}

/**
  * @brief Initializes IIR filter
  */
void IIR_Filter_Init(arm_biquad_casd_df1_inst_f32 *pfilter, uint8_t numStages, float32_t *pcoeffs, float32_t *pstate)
{
  arm_biquad_cascade_df1_init_f32(pfilter, numStages, pcoeffs, pstate);
}

/**
  * @brief  Update IIR filter output value
  */
void IIR_Filter(arm_biquad_casd_df1_inst_f32 *pfilter, float32_t *psrc, float32_t *pdst)
{
  arm_biquad_cascade_df1_f32(pfilter, psrc, pdst, 1);
}

/**
  * @brief  Integrates input value
  */
void Integrator(float_t *in, float_t *out, float_t *mem)
{
  mem[0] = mem[0] + (in[0] * (1.0f / (float_t)sensor_hub_data.data_rate_Hz));
  out[0] = mem[0];
}

/**
  * @brief  Derivates input value
  */
void Derivator(float_t *in, float_t *out, float_t *mem)
{

  out[0] = (in[0] - mem[0]) / (1.0f / (float_t)sensor_hub_data.data_rate_Hz);
  mem[0] = in[0];
}

/**
  * @brief  Switches between two integer input values
  */
void SignalSwitchInt(int32_t *in1, int32_t *in2, int32_t *control, int32_t *out)
{
  if (control[0] == 0)
  {
    out[0] = in1[0];
  }
  else
  {
    out[0] = in2[0];
  }
}

/**
  * @brief  Switches between two float_t input values
  */

void SignalSwitchFloat(float_t *in1, float_t *in2, int32_t *control, float_t *out)
{
  if (control[0] == 0)
  {
    out[0] = in1[0];
  }
  else
  {
    out[0] = in2[0];
  }
}

/**
  * @brief  Calculates signal features
  */
void Feature_Computation(float_t *in, int32_t *reset, float_t *max, float_t *min, float_t *mean, float_t *var,
                         float_t *peak, float_t *energy, uint32_t length, int32_t *mem, float_t *buffer)
{
  /* mem[0] ... index to the buffer */
  /* mem[1] ... buffer full flag */

  float_t mean_temp = 0.0f;
  float_t var_temp = 0.0f;
  float_t energy_temp = 0.0f;

  float_t imax;
  float_t imin;
  float_t imean;
  float_t tmp;

  max[0] = 0.0f;
  min[0] = 0.0f;
  mean[0] = 0.0f;
  var[0] = 0.0f;
  peak[0] = 0.0f;
  energy[0] = 0.0f;

  /* Clear buffer if reset occurs */
  if (reset[0] != 0)
  {
    for (uint32_t i = 0; i < length; i++)
    {
      buffer[i] = 0.0f;
    }

    mem[0] = 0;
    mem[1] = 0;

    return;
  }

  /* Add value into buffer */
  buffer[mem[0]] = in[0];
  mem[0] = (mem[0] + 1) % (int32_t)length;

  /* Set flag if buffer is full */
  if ((mem[1] == 0) && (mem[0] == 0))
  {
    mem[1] = 1;
  }

  /* If buffer is full calculate the features */
  if (mem[1] == 1)
  {
    imax = 1.175494e-38f;
    imin = 3.402823e+38f;

    for (uint32_t i = 0; i < length; i++)
    {
      if (imax < buffer[i]) { imax = buffer[i]; }
      if (imin > buffer[i]) { imin = buffer[i]; }
      mean_temp += buffer[i];
      energy_temp += buffer[i] * buffer[i];
    }

    imean = mean_temp / (float_t)length;

    for (uint32_t j = 0; j < length; j++)
    {
      tmp = (buffer[j] - imean);
      var_temp += tmp * tmp;
    }

    max[0] = imax;
    min[0] = imin;
    mean[0] = imean;
    var[0] = var_temp / (float_t)length;
    peak[0] = imax - imin;
    energy[0] = energy_temp;
  }
}

/**
  * @brief  Delays input signal to the output
  */
void Signal_Delay(float_t *in, float_t *out, uint32_t length, float_t *mem1, int32_t *mem2)
{
  out[0] = mem1[(mem2[0] + 1) % (int32_t)length];
  mem1[mem2[0]] = in[0];
  mem2[0] = (mem2[0] + 1) % (int32_t)length;
}

/**
  * @brief  Initialize data buffer for feature calculation
  */
void Feature_Computation_2_Init(float_t *buffer, int32_t n_items)
{
  (void)memset(buffer, 0, (uint32_t)n_items * sizeof(float_t));
}

/**
  * @brief  Calculates signal features
  * increased speed (one-pass) and accuracy (compensated summation), samples are not buffered, computation is spread
  * evenly
  */
void Feature_Computation_2(float_t *in, int32_t *reset, float_t *max, float_t *min, float_t *mean, float_t *var,
                           float_t *peak2peak, float_t *energy, int32_t *updated, uint32_t length, int32_t *mem,
                           float_t *buffer)
{
  /*
  mem[0] is the index to the buffer, mem[1] is the buffer full flag

  buffer for samples is re-purposed to store intermediate results
  buffer[0] = max
  buffer[1] = min
  buffer[2] = mean
  buffer[3] = var
  buffer[4] = energy
  buffer[5] = resmeanincr
  buffer[6] = resvarincr
  buffer[7] = max - previous value from whole buffer
  buffer[8] = min - previous value from whole buffer
  buffer[9] = mean - previous value from whole buffer
  buffer[10] = var - previous value from whole buffer
  buffer[11] = energy - previous value from whole buffer
  buffer[12] = peak2peak - previous value from whole buffer
  */

  if (*reset != 0)
  {
    mem[1] = 0;
    mem[0] = mem[1];
    (void)memset(buffer, 0, 13U * sizeof(float_t));
    *energy = 0.0f;
    *peak2peak = *energy;
    *var = *peak2peak;
    *mean = *var;
    *min = *mean;
    *max = *min;
    *updated = 0;
    return;
  }

  int32_t idx = mem[0];
  mem[0] = (idx + 1) % (int32_t)length;

  float_t t = (*in);
  if (idx == 0)
  {
    buffer[2] = t;         /* tmax = tmin = tmean = t where t is 1st sample */
    buffer[1] = buffer[2]; /* tmax = tmin = tmean = t where t is 1st sample */
    buffer[0] = buffer[1]; /* tmax = tmin = tmean = t where t is 1st sample */
    buffer[3] = 0.0f;      /* tvar = 0.0 */
    buffer[4] = t * t;     /* tenergy = t*t */
    buffer[6] = 0.0f;      /* resmeanincr = 0.0, resvarincr = 0.0, residual increment for mean and var */
    buffer[5] = buffer[6]; /* resmeanincr = 0.0, resvarincr = 0.0, residual increment for mean and var */
  }

  if (idx > 0)
  {
    idx++;
    float_t n = (float_t)idx;

    /* one-pass algorithm with Kahan compensated summation */
    volatile float_t old, incr, totincr; /* volatile to block unwanted optimizations */
    float_t tmp;

    if (t > buffer[0]) /* max */
    {
      buffer[0] = t;
    }

    if (t < buffer[1]) /* min */
    {
      buffer[1] = t;
    }

    buffer[4] += t * t; /* energy */

    old = buffer[2]; /* mean */
    incr = ((t - old) - buffer[5]) / n; /* increment is d = (t-mean)/n but here mean = tmean+resmeanincr */
    totincr = incr + buffer[5];     /* total target increment to be applied is incr + residual stored in resmeanincr */
    buffer[2] += totincr;           /* because of rounding actual increment is not equal to target increment */
    buffer[5] = (old - buffer[2]);  /* residual increment                                  */
    buffer[5] += totincr;           /*                    is target minus actual increment */

    old = buffer[3]; /* var */

    tmp = incr;                         /* */
    tmp *= incr * (n * (n - 1.0f));     /* */
    incr = ((tmp - old) - buffer[6]) / n; /* increment is d = (d*d*(n-1)-var/n) but here var = tvar+resvarincr */

    totincr = incr + buffer[6]; /* total target increment to be applied is incr + residual stored in resvarincr */
    buffer[3] += totincr; /* because of rounding actual increment is not equal to target increment */
    buffer[6] = (old - buffer[3]); /* */
    buffer[6] += totincr;          /* residual increment is target minus actual increment */
  }

  if (idx == ((int32_t)length - 1))
  {
    mem[1] = 1;
    buffer[7] = buffer[0];
    *max = buffer[7];        /* max */
    buffer[8] = buffer[1];
    *min = buffer[8];        /* min */
    buffer[9] = buffer[2];
    *mean = buffer[9];       /* mean */
    buffer[10] = buffer[3];
    *var = buffer[10];       /* var */
    buffer[11] = buffer[4];
    *energy = buffer[11];    /* energy */
    buffer[12] = buffer[0] - buffer[1];
    *peak2peak = buffer[12]; /* peak2peak */
    *updated = 1;
  }
  else
  {
    *max = buffer[7];        /* max */
    *min = buffer[8];        /* min */
    *mean = buffer[9];       /* mean */
    *var = buffer[10];       /* var */
    *energy = buffer[11];    /* energy */
    *peak2peak = buffer[12]; /* peak2peak */
    *updated = 0;
  }
}

/**
  * @brief  Detects zero crossings
  * if first or second sample is at zero, then zero crossing is NOT detected!
  */
void Zero_Crossing(float_t *in, int32_t *out, uint32_t edge, float_t *mem)
{
  /* *in is current new sample, *mem is previous older sample */
  int8_t negative_zerocrossing = (int8_t)(((*mem) > 0.0f) && ((*in) < 0.0f));
  int8_t positive_zerocrossing = (int8_t)(((*mem) < 0.0f) && ((*in) > 0.0f));
  int8_t detected;

  switch (edge)
  {
    case AB_FALLING_EDGE:
      detected = negative_zerocrossing;
      break;

    case AB_RISING_EDGE:
      detected = positive_zerocrossing;
      break;

    default:
      detected = (int8_t)((negative_zerocrossing != 0) || (positive_zerocrossing != 0));
      break;
  }

  (*out) = (int32_t)detected;
  (*mem) = (*in);
}

/**
  * @brief  Detects zero crossings with hysteresis
  * hysteresis allows to reject crossings due to noise
  */
void Zero_Crossing_Hyst(float_t *in, int32_t *out, uint32_t edge, float_t hyst, int32_t *stat)
{
  /* *in is current new sample, *stat is status */
  int8_t negative_zerocrossing = (int8_t)((*in) < -hyst); /* check if going below low threshold */
  int8_t positive_zerocrossing = (int8_t)((*in) > +hyst); /* check if going above high threshold */
  int8_t detected = 0; /* default is no event */

  if ((negative_zerocrossing != 0) && (*stat != AB_RISING_EDGE)) /* waiting to go below low threshold */
  {
    if (edge != (uint32_t)AB_RISING_EDGE)
    {
      detected = 1; /* signal event if wanted (that is: not only rising wanted) */
    }

    *stat = AB_RISING_EDGE; /* then wait to go above high threshold */
  }

  if ((positive_zerocrossing != 0) && (*stat != AB_FALLING_EDGE)) /* waiting to go above high threshold */
  {
    if (edge != (uint32_t)AB_FALLING_EDGE)
    {
      detected = 1; /* signal event if wanted (that is: not only falling wanted) */
    }

    *stat = AB_FALLING_EDGE; /* then wait to go below low threshold */
  }

  (*out) = (int32_t)detected;
}

/**
  * @brief  Detects peeks
  * noise may trigger peak detection as soon as signal is above threshold
  */
void Peak_Detector(float_t *in, int32_t *out, uint32_t peak, float_t threshold, float_t *mem)
{
  /* *in is current new sample, mem[1] is previous older, mem[0] is previous oldest */
  int8_t negative_peak;
  int8_t positive_peak;
  int8_t detected = (int8_t)(fabs(*in) > threshold); /* to process all samples threshold must be negative! */

  if (detected != 0)
  {
    negative_peak = (int8_t)((mem[1] < mem[0]) && (mem[1] < (*in))); /* mem[1] is lower */
    positive_peak = (int8_t)((mem[1] > mem[0]) && (mem[1] > (*in))); /* mem[1] is higher */

    switch (peak)
    {
      case NEGATIVE_PEAK:
        detected = negative_peak;
        break;

      case POSITIVE_PEAK:
        detected = positive_peak;
        break;

      default:
        detected = (int8_t)((negative_peak != 0) || (positive_peak != 0));
        break;
    }
  }

  (*out) = (int32_t)detected;

  mem[0] = mem[1];
  mem[1] = (*in);
}

/**
  * @brief  Detects peeks
  * true peak detector with relative (not absolute) threshold
  */
void Peak_Detector_RelTh(float_t *in, int32_t *out, uint32_t peak, float_t threshold, int32_t reset, int32_t *stat,
                         float_t *mem)
{
  float_t t = (*in);
  int8_t negative_peak = 0;
  int8_t positive_peak = 0;
  int8_t detected;

  /* *in is current new sample, mem[0] is max, mem[1] is min */
  if (reset != 0)
  {
    *stat = 0;
    mem[0] = -FLT_MAX;
    mem[1] = +FLT_MAX;
    *out = 0;
    return;
  }

  if (*stat == 0)
  {
    /* at startup unknown search status */
    if (t > mem[0])
    {
      mem[0] = t;
    }

    if (t < mem[1])
    {
      mem[1] = t;
    }

    if ((mem[0] - t) >= threshold)
    {
      /* going down, look for min */
      *stat = NEGATIVE_PEAK;
      mem[1] = +FLT_MAX;
    }

    if ((t - mem[1]) >= threshold)
    {
      /* going up, look for max */
      *stat = POSITIVE_PEAK;
      mem[0] = -FLT_MAX;
    }
  }

  if (*stat == POSITIVE_PEAK)
  {
    if (t > mem[0])
    {
      mem[0] = t;
    }

    if ((mem[0] - t) >= threshold)
    {
      positive_peak = 1;
      *stat = NEGATIVE_PEAK;
      mem[1] = +FLT_MAX;
    }
  }
  else
  {
    /* stat == NEGATIVE_PEAK */
    if (t < mem[1])
    {
      mem[1] = t;
    }

    if ((t - mem[1]) >= threshold)
    {
      negative_peak = 1;
      *stat = POSITIVE_PEAK;
      mem[0] = -FLT_MAX;
    }
  }

  switch (peak)
  {
    case NEGATIVE_PEAK:
      detected = negative_peak;
      break;

    case POSITIVE_PEAK:
      detected = positive_peak;
      break;

    default:
      detected = (int8_t)((negative_peak != 0) || (positive_peak != 0));
      break;
  }

  (*out) = (int32_t)detected;
}

/**
  * @brief  Detects spikes
  * noise is rejected by higher threshold
  */
void Spike_Detector(float_t *in, int32_t *out, uint32_t peak, float_t threshold, float_t *mem)
{
  /* *in is current new sample, mem[1] is previous older, mem[0] is previous oldest */
  float_t left_diff = (*in) - mem[1];
  float_t right_diff = mem[1] - mem[0];
  int8_t negative_peak = (int8_t)((right_diff < -threshold) && (left_diff > +threshold));  /* mem[1] is lower */
  int8_t positive_peak = (int8_t)((right_diff > +threshold) && (left_diff < -threshold));  /* mem[1] is higher */
  int8_t detected;

  switch (peak)
  {
    case NEGATIVE_PEAK:
      detected = negative_peak;
      break;

    case
        POSITIVE_PEAK:
      detected = positive_peak;
      break;

    default:
      detected = (int8_t)((negative_peak != 0) || (positive_peak != 0));
      break;
  }

  (*out) = (int32_t)detected;

  mem[0] = mem[1];
  mem[1] = (*in);
}

/**
  * @}
  */

/**
  * @}
  */
