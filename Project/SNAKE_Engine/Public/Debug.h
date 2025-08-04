#pragma once

#include <iostream>
#include <cstring>

#define SNAKE_DEBUG
#define SKIP_LOG
#define SKIP_WRN
//#define SKIP_ERR


/**
 * @file Debug.h
 * @brief Logging and debugging macro definitions for the SNAKE engine.
 *
 * @details
 * This file defines macros for standardized logging output in debug builds, including:
 * - SNAKE_LOG: general purpose log messages
 * - SNAKE_WRN: warning messages
 * - SNAKE_ERR: error messages
 *
 * Logging macros include the filename and line number of the call site.
 * You can selectively disable specific levels of output by defining:
 * - SKIP_LOG: disables SNAKE_LOG
 * - SKIP_WRN: disables SNAKE_WRN
 * - SKIP_ERR: disables SNAKE_ERR
 *
 * These macros are no-ops in builds unless SNAKE_DEBUG is defined.
 *
 * Example usage:
 * @code
 * SNAKE_LOG("Game started");
 * SNAKE_WRN("Invalid tag used");
 * SNAKE_ERR("Failed to load shader");
 * @endcode
 */


#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#if defined(SNAKE_DEBUG) && !defined(SKIP_LOG)
#define SNAKE_LOG(x) std::cout << "[LOG] " << FILENAME << ":" << __LINE__ << " - " << x << std::endl
#else
#define SNAKE_LOG(x) do {} while(0)
#endif

#if defined(SNAKE_DEBUG) && !defined(SKIP_WRN)
#define SNAKE_WRN(x) std::cerr << "[WRN] " << FILENAME << ":" << __LINE__ << " - " << x << std::endl
#else
#define SNAKE_WRN(x) do {} while(0)
#endif

#if defined(SNAKE_DEBUG) && !defined(SKIP_ERR)
#define SNAKE_ERR(x) std::cerr << "[ERR] " << FILENAME << ":" << __LINE__ << " - " << x << std::endl
#else
#define SNAKE_ERR(x) do {} while(0)
#endif
