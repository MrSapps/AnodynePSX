#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::Logging {

enum class LogLevel {
    Debug = 0,
    Info,
    Warning,
    Error,
    Critical
};

struct LogLine {
    LogLevel Level = LogLevel::Info;
    std::string Message;
    LogLine() = default;
    LogLine(LogLevel l, const std::string& m) : Level(l), Message(m) {}
};

class DebugLogger {
public:
    static void AddDebug(const std::string& message, bool showStack = false);
    static void AddInfo(const std::string& message, bool showStack = false);
    static void AddWarning(const std::string& message, bool showStack = false);
    static void AddError(const std::string& message, bool showStack = true);
    static void AddCritical(const std::string& message, bool showStack = true);
    static void AddException(const std::exception& ex);
    static LogLine Read();
    static LogLine Read(LogLevel level);
    static LogLine Read(int index);
    static LogLine Read(LogLevel level, int index);

private:
    static std::string _logPath;
    static std::queue<LogLine> _debugLog;
    static int _maxLogs;
    static void AddLine(const LogLine& line, bool showStack);
    static void FormatLine(LogLine& line);
};

} // namespace AnodyneSharp::Logging

using AnodyneSharp::Logging::DebugLogger;
using AnodyneSharp::Logging::LogLevel;
using AnodyneSharp::Logging::LogLine;
