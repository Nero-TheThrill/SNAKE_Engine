#pragma once
#include <iostream>
#include <sstream>
#pragma once

#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define SNAKE_LOG(x)   do { std::ostringstream oss; oss << x; DebugLogger::Log(oss.str(), FILENAME, __LINE__); } while(0)
#define SNAKE_WRN(x)   do { std::ostringstream oss; oss << x; DebugLogger::Warn(oss.str(), FILENAME, __LINE__); } while(0)
#define SNAKE_ERR(x)   do { std::ostringstream oss; oss << x; DebugLogger::Error(oss.str(), FILENAME, __LINE__); } while(0)


enum class LogLevel
{
    None,
    Error,
    Warning,
    Log,
    All
};

class DebugLogger
{
public:
    static void SetLogLevel(LogLevel level);
    static LogLevel GetLogLevel();

    static void Log(const std::string& msg, const char* file, int line);
    static void Warn(const std::string& msg, const char* file, int line);
    static void Error(const std::string& msg, const char* file, int line);

private:
    static LogLevel currentLevel;
};