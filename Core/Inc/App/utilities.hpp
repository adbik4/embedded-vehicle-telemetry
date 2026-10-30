#pragma once
#include "cmsis_os2.h"
#include "settings.h"
#include <cstdio>

/* MACROS */
#if !defined(NDEBUG) && !defined(UNIT_TEST)
#define PRINT_DEBUG(...) do { \
  char debug_msg_tx_buf[DEBUG_MSG_SIZE + 1]; \
  snprintf(debug_msg_tx_buf, DEBUG_MSG_SIZE, __VA_ARGS__); \
  osMessageQueuePut(ComDebugQueueHandle, debug_msg_tx_buf, 0, 1000); \
} while (0)
#else
#define PRINT_DEBUG(...)
#endif

/* CLASSES */
template <class T>
class Singleton {
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }

    ~Singleton() = default;

protected:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

/* FUNCTIONS */
void Quaternion2Euler(const float *q, float *out);