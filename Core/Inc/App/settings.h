#pragma once

#define USERNAME_A "Adrian"
#define USERNAME_B "Natalia"

#define DISPLAY_REFRESH_RATE_MS 5000
#define BUTTON_HOLD_THRESH_MS 500
#define DEBOUNCE_MS 10

#define SYSTEM_TASK_STACK_SIZE          (1024)   // optimal - ??
#define COM_DEBUG_TASK_STACK_SIZE       (185)   // optimal - 185
#define SENSOR_HUB_TASK_STACK_SIZE      (1024)   // optimal - ??
#define GNSS_TASK_STACK_SIZE            (1024)   // optimal - ??
#define DISPLAY_TASK_STACK_SIZE         (1024)   // optimal - ??
#define INPUT_MANAGER_TASK_STACK_SIZE   (1024)   // optimal - ??

#define SYSTEM_TASK_PRIO        ((osPriority_t) osPriorityRealtime)
#define COM_DEBUG_TASK_PRIO     ((osPriority_t) osPriorityHigh)
#define SENSOR_HUB_TASK_PRIO    ((osPriority_t) osPriorityNormal)
#define GNSS_TASK_PRIO          ((osPriority_t) osPriorityLow)
#define DISPLAY_TASK_PRIO       ((osPriority_t) osPriorityLow1)
#define INPUT_MANAGER_TASK_PRIO ((osPriority_t) osPriorityHigh1)

#define DEBUG_QUEUE_LENGTH      (16)
#define DEBUG_MSG_SIZE          (256)