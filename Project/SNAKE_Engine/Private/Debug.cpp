#include "Debug.h"

LogLevel DebugLogger::currentLevel = LogLevel::Log;

void DebugLogger::SetLogLevel(LogLevel level)
{
    currentLevel = level;
}

LogLevel DebugLogger::GetLogLevel()
{
    return currentLevel;
}

void DebugLogger::Log(const std::string& msg, const char* file, int line)
{
    if (currentLevel >= LogLevel::Log)
        std::cout << "[LOG] " << file << ":" << line << " - " << msg << std::endl;
}

void DebugLogger::Warn(const std::string& msg, const char* file, int line)
{
    if (currentLevel >= LogLevel::Warning)
        std::cerr << "[WRN] " << file << ":" << line << " - " << msg << std::endl;
}

void DebugLogger::Error(const std::string& msg, const char* file, int line)
{
    if (currentLevel >= LogLevel::Error)
        std::cerr << "[ERR] " << file << ":" << line << " - " << msg << std::endl;
}