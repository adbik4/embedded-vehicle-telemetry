#pragma once

#if defined(UNIT_TEST)
    using osMessageQueueId_t = void*;
#else
    #include "cmsis_os2.h"
#endif

#include "etl/string.h"
#include "settings.h"
#include "motion_fx.h"
#include <cstdio>

extern osMessageQueueId_t ComDebugQueueHandle;

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

template <typename T>
struct is_etl_string : std::false_type {};

template <size_t N>
struct is_etl_string<etl::string<N>> : std::true_type {};

template <size_t N>
etl::string<N> magcal_to_string(const MFX_MagCal_quality_t& cal) {
    switch (cal) {
        case MFX_MAGCALUNKNOWN:
            return "UNKNOWN";
        case MFX_MAGCALPOOR:
            return "POOR";
        case MFX_MAGCALOK:
            return "OK";
        case MFX_MAGCALGOOD:
            return "GOOD";
        default:
            return "";
    }
}