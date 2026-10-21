#pragma once
#include "settings.h"

char debug_msg_tx_buf[DEBUG_MSG_SIZE];

/* MACROS */
#ifndef NDEBUG
#define PRINT_DEBUG(...) do { \
  snprintf(debug_msg_tx_buf, sizeof(debug_msg_tx_buf), __VA_ARGS__); \
  osMessageQueuePut(ComDebugQueueHandle, debug_msg_tx_buf, 0, 1000); \
} while (0)
#else
#define PRINT_DEBUG(...)
#endif

/* FUNCTIONS */

void Quaternion2Euler(float *q, float *out);