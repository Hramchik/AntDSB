//
// Created by sanek on 06/12/2025.
//

#ifndef ANTDSB_LOGGER_H
#define ANTDSB_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

void LogInit(const std::string& log_dir = "logs");

void Log(LogLevel level, const std::string& message);

inline void LogDebug(const std::string& msg)   { Log(LogLevel::Debug,   msg); }
inline void LogInfo(const std::string& msg)    { Log(LogLevel::Info,    msg); }
inline void LogWarn(const std::string& msg)    { Log(LogLevel::Warning, msg); }
inline void LogError(const std::string& msg)   { Log(LogLevel::Error,   msg); }

void LogChannel(long long channel_id, const std::string& channel_name, const std::string& message);

#endif //ANTDSB_LOGGER_H