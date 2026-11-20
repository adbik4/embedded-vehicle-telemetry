#pragma once

#define USERNAME_A "User1"
#define USERNAME_B "User2"

#define DISPLAY_REFRESH_RATE_MS (100)
#define DEBOUNCE_MS (6)

#define SYSTEM_TASK_STACK_SIZE          (64)   // optimal - ??
#define COM_EXPORT_TASK_STACK_SIZE      (1024)   // optimal - ??
#define COM_DEBUG_TASK_STACK_SIZE       (100)   // optimal - 100
#define SENSOR_HUB_TASK_STACK_SIZE      (865)   // optimal - 865
#define GNSS_TASK_STACK_SIZE            (225)   // optimal - 225
#define DISPLAY_TASK_STACK_SIZE         (1536)   // optimal - 1536
#define INPUT_MANAGER_TASK_STACK_SIZE   (62)   // optimal - 62

#define SYSTEM_TASK_PRIO        ((osPriority_t) osPriorityNormal)
#define COM_EXPORT_TASK_PRIO    ((osPriority_t) osPriorityLow3)
#define COM_DEBUG_TASK_PRIO     ((osPriority_t) osPriorityLow2)
#define SENSOR_HUB_TASK_PRIO    ((osPriority_t) osPriorityHigh)
#define GNSS_TASK_PRIO          ((osPriority_t) osPriorityLow)
#define DISPLAY_TASK_PRIO       ((osPriority_t) osPriorityLow1)
#define INPUT_MANAGER_TASK_PRIO ((osPriority_t) osPriorityHigh2)

#define DEBUG_QUEUE_LENGTH      (16)
#define DEBUG_MSG_SIZE          (100)
#define TELEMETRY_MSG_SIZE      (512)

#define SCREEN_WIDTH    (296)
#define SCREEN_HEIGHT   (152)

constexpr char MAGIC_WORD[] ("EXPORT_TLM\r\n");
#define MAGIC_WORD_SIZE strlen(MAGIC_WORD)