#pragma once
#include <iostream>
#include <sstream>
#pragma once

/**
 * @def FILENAME
 * @brief Extracts the basename from __FILE__ using '\\' as a separator.
 * @details If a backslash is found, returns the substring after it; otherwise returns __FILE__ unchanged.
 */
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

 /**
  * @def SNAKE_LOG
  * @brief Logs a message at Log level with file name and line number.
  * @details Wraps the expression into an ostringstream, then calls DebugLogger::Log(..., FILENAME, __LINE__).
  */
#define SNAKE_LOG(x)   do { std::ostringstream oss; oss << x; DebugLogger::Log(oss.str(), FILENAME, __LINE__); } while(0)

  /**
   * @def SNAKE_WRN
   * @brief Logs a message at Warning level with file name and line number.
   * @details Calls DebugLogger::Warn(..., FILENAME, __LINE__).
   */
#define SNAKE_WRN(x)   do { std::ostringstream oss; oss << x; DebugLogger::Warn(oss.str(), FILENAME, __LINE__); } while(0)

   /**
    * @def SNAKE_ERR
    * @brief Logs a message at Error level with file name and line number.
    * @details Calls DebugLogger::Error(..., FILENAME, __LINE__).
    */
#define SNAKE_ERR(x)   do { std::ostringstream oss; oss << x; DebugLogger::Error(oss.str(), FILENAME, __LINE__); } while(0)

    /**
     * @brief Severity filter for logging output.
     * @details
     * - Ordering (low->high): None, Error, Warning, Log, All.
     * - Messages are emitted when currentLevel >= message level.
     * - Default level is Log (see Debug.cpp).
     */
enum class LogLevel
{
    None,    ///< No messages are emitted.
    Error,   ///< Only Error messages pass.
    Warning, ///< Warning and Error pass.
    Log,     ///< Log, Warning, and Error pass.
    All      ///< All messages pass.
};

/**
 * @brief Static logger with level filtering and file/line tagging.
 *
 * @details
 * - SetLogLevel/GetLogLevel control the global filter.
 * - Output streams: Log -> std::cout, Warn/Error -> std::cerr.
 * - Log/Warn/Error prepend tags "[LOG]"/"[WRN]"/"[ERR]" and include "file:line - message".
 */
class DebugLogger
{
public:
    /** @brief Sets the global log level filter. */
    static void SetLogLevel(LogLevel level);
    /** @brief Returns the current global log level. */
    static LogLevel GetLogLevel();

    /** @brief Emits a log message if currentLevel >= LogLevel::Log. */
    static void Log(const std::string& msg, const char* file, int line);
    /** @brief Emits a warning if currentLevel >= LogLevel::Warning. */
    static void Warn(const std::string& msg, const char* file, int line);
    /** @brief Emits an error if currentLevel >= LogLevel::Error. */
    static void Error(const std::string& msg, const char* file, int line);

private:
    /** @brief Current global log level (default: Log). */
    static LogLevel currentLevel;
};
