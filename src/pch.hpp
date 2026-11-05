#pragma once

// stl
#include <cmath>
#include <algorithm>
#include <array>
#include <cstddef>
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <variant>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

// raylib
#include <raylib.h>

#if DEBUG
#include "raygui.h"
#endif

#if defined(__clang__) || defined(__GNUC__)
extern "C" void TraceLog(int logLevel, const char *text, ...)
    __attribute__((format(printf, 2, 3)));
#endif

// macro
#define LOGINFO(fmt, ...)  TraceLog(LOG_INFO,    fmt, ##__VA_ARGS__)
#define LOGWARN(fmt, ...)  TraceLog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define LOGERROR(fmt, ...) TraceLog(LOG_ERROR,   fmt, ##__VA_ARGS__)

#if defined(__clang__)
    #define DEBUG_BREAK() __builtin_debugtrap()
#elif defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__)
    #define DEBUG_BREAK() __builtin_trap()
#else
    #define DEBUG_BREAK() ((void)0)
#endif

#if defined(NDEBUG)
    #define ASSERT(cond)            ((void)0)
    #define ASSERTF(cond, fmt, ...) ((void)0)
#else
    #define ASSERT(cond)                                                    \
        do {                                                                \
            if (!(cond)) {                                                  \
                LOGERROR("ASSERT: %s (%s:%d)",                              \
                    #cond, __FILE__, __LINE__);                             \
                DEBUG_BREAK();                                              \
            }                                                               \
        } while (0)

    #define ASSERTF(cond, fmt, ...)                                         \
        do {                                                                \
            if (!(cond)) {                                                  \
                LOGERROR("ASSERT: %s (%s:%d) — " fmt,                       \
                    #cond, __FILE__, __LINE__, ##__VA_ARGS__);              \
                DEBUG_BREAK();                                              \
            }                                                               \
        } while (0)
#endif

#define UNREACHABLE()                                                       \
    do {                                                                    \
        TraceLog(LOG_FATAL, "UNREACHABLE: %s:%d", __FILE__, __LINE__);      \
        DEBUG_BREAK();                                                      \
    } while (0)

#if !defined(PROJECT_NAME)
#define PROJECT_NAME "A game"
#endif

// core
#include "math.hpp"
#include "input.hpp"
#include "texture.hpp"
