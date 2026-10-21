#pragma once

#define SYSTEM_TASK_STACK_SIZE      (2048)   // optimal - ??
#define COM_DEBUG_TASK_STACK_SIZE   (185)   // optimal - 185
#define SENSOR_HUB_TASK_STACK_SIZE  (2048)   // optimal - ??
#define GNSS_TASK_STACK_SIZE        (2048)   // optimal - ??

#define SYSTEM_TASK_PRIO        ((osPriority_t) osPriorityRealtime)
#define COM_DEBUG_TASK_PRIO     ((osPriority_t) osPriorityHigh)
#define SENSOR_HUB_TASK_PRIO    ((osPriority_t) osPriorityNormal)
#define GNSS_TASK_PRIO          ((osPriority_t) osPriorityLow)

#define DEBUG_QUEUE_LENGTH      (16)
#define DEBUG_MSG_SIZE          (256)