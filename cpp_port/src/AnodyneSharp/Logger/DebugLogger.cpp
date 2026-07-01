#include "AnodyneSharp/Logger/DebugLogger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace AnodyneSharp::Logging {

std::string DebugLogger::_logPath;
std::queue<LogLine> DebugLogger::_debugLog;
int DebugLogger::_maxLogs = 10;

void DebugLogger::AddDebug(const std::string& message, bool showStack) {
    AddLine(LogLine{LogLevel::Debug, message}, showStack);
}
void DebugLogger::AddInfo(const std::string& message, bool showStack) {
    AddLine(LogLine{LogLevel::Info, message}, showStack);
}
void DebugLogger::AddWarning(const std::string& message, bool showStack) {
    AddLine(LogLine{LogLevel::Warning, message}, showStack);
}
void DebugLogger::AddError(const std::string& message, bool showStack) {
    AddLine(LogLine{LogLevel::Error, message}, showStack);
}
void DebugLogger::AddCritical(const std::string& message, bool showStack) {
    AddLine(LogLine{LogLevel::Critical, message}, showStack);
}
void DebugLogger::AddException(const std::exception& ex) {
    AddLine(LogLine{LogLevel::Error, ex.what()}, false);
}

LogLine DebugLogger::Read() {
    if (_debugLog.empty()) return {};
    auto copy = _debugLog;
    LogLine last;
    while (!copy.empty()) { last = copy.front(); copy.pop(); }
    return last;
}
LogLine DebugLogger::Read(LogLevel level) {
    auto copy = _debugLog;
    LogLine found;
    while (!copy.empty()) {
        if (copy.front().Level == level) found = copy.front();
        copy.pop();
    }
    return found;
}
LogLine DebugLogger::Read(int index) {
    auto copy = _debugLog;
    int i = 0;
    while (!copy.empty()) {
        if (i++ == index) return copy.front();
        copy.pop();
    }
    return {};
}
LogLine DebugLogger::Read(LogLevel level, int index) {
    auto copy = _debugLog;
    int i = 0;
    while (!copy.empty()) {
        if (copy.front().Level == level && i++ == index) return copy.front();
        copy.pop();
    }
    return {};
}

void DebugLogger::AddLine(const LogLine& logLine, bool showStack) {
    if ((int)_debugLog.size() == _maxLogs) _debugLog.pop();
    LogLine line = logLine;
    FormatLine(line);
    _debugLog.push(line);
#ifdef DEBUG
    std::cout << line.Message << std::endl;
#endif
}

void DebugLogger::FormatLine(LogLine& line) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    static const char* levelNames[] = {"Debug","Info","Warning","Error","Critical"};
    line.Message = oss.str() + " | " + levelNames[(int)line.Level] + " | " + line.Message;
}

} // namespace AnodyneSharp::Logging
