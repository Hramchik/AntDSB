//
// Created by sanek on 06/12/2025.
//

#include "Logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <map>

namespace {
    std::mutex g_log_mutex;
    std::ofstream g_log_file;
    bool g_log_inited = false;
    std::string g_log_dir;

    std::string MakeTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
    #if defined(_WIN32)
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

    std::string MakeLogFileName() {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        #if defined(_WIN32)
            localtime_s(&tm, &t);
        #else
            localtime_r(&t, &tm);
        #endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d.%m.%Y") << "_Discord.log";
        return oss.str();
    }

    const char* LevelToStr(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
        }
        return "UNKWN";
    }

#include <cctype>

    std::string SanitizeFilePart(const std::string& name) {
        std::string s;
        s.reserve(name.size());

        for (unsigned char ch : name) {
            if (ch == '\\' || ch == '/' || ch == ':' || ch == '*' ||
                ch == '?'  || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
                    continue;
                }

            if ((ch >= '0' && ch <= '9') ||
                (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                ch == ' ' || ch == '-' || ch == '_' || ch == '.') {
                    s.push_back(static_cast<char>(ch));
                    continue;
                }
        }

        if (s.empty())
            s = "unknown";

        return s;
    }


    std::string MakeChannelLogFileName() {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        #if defined(_WIN32)
            localtime_s(&tm, &t);
        #else
            localtime_r(&t, &tm);
        #endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d.%m.%Y") << "_Channel.log";

        return oss.str();
    }
}

void LogInit(const std::string& log_dir) {
    {
        std::lock_guard<std::mutex> lock(g_log_mutex);
        if (g_log_inited) return;

        g_log_dir = log_dir;
        std::error_code ec;
        std::filesystem::create_directories(g_log_dir, ec);

        std::string path = g_log_dir + "/" + MakeLogFileName();
        g_log_file.open(path, std::ios::app);
        if (!g_log_file.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << path << "\n";
            return;
        }

        g_log_inited = true;
        std::cout << "[Logger] Initialized, file: " << path << "\n";
    }

    Log(LogLevel::Info, "Logger initialized");
}

void Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_log_mutex);

    std::string ts = MakeTimestamp();
    const char* lvl = LevelToStr(level);

    std::ostringstream line;
    line << "[" << ts << "] [" << lvl << "] " << message << "\n";
    std::string s = line.str();

    std::cout << s;

    if (g_log_inited && g_log_file.is_open()) {
        g_log_file << s;
        g_log_file.flush();
    }
}

void LogChannel(long long channel_id,
                const std::string& channel_name,
                const std::string& message)
{
    std::lock_guard<std::mutex> lock(g_log_mutex);

    const std::string chan_dir = g_log_dir + "/channels/" + std::to_string(channel_id);
    std::error_code ec;
    std::filesystem::create_directories(chan_dir, ec);

    const std::string file_name = MakeChannelLogFileName();  // БЕЗ имени канала
    const std::string full_path = chan_dir + "/" + file_name;

    const std::string ts = MakeTimestamp();
    std::ostringstream line;
    line << "[" << ts << "] " << message << "\n";
    const std::string s = line.str();

    std::cout << "[CHAN " << channel_id << "] " << s;

    static std::map<std::string, std::ofstream> channel_files;
    auto& f = channel_files[full_path];
    if (!f.is_open()) {
        f.open(full_path, std::ios::app);
        if (!f.is_open()) {
            std::cerr << "[Logger] Failed to open channel log: " << full_path << "\n";
            return;
        }
    }

    f << s;
    f.flush();
}
