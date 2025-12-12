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
#include <zlib.h>

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

    // Парсим дату из имени вида "dd.mm.yyyy_Discord.log" или "dd.mm.yyyy_Channel.log"
    bool ParseDateFromLogName(const std::string& filename, std::tm& out_tm) {
        if (filename.size() < 10) return false;
        try {
            int d = std::stoi(filename.substr(0, 2));
            int m = std::stoi(filename.substr(3, 2));
            int y = std::stoi(filename.substr(6, 4));

            std::tm tm{};
            tm.tm_mday = d;
            tm.tm_mon  = m - 1;
            tm.tm_year = y - 1900;
            out_tm = tm;
            return true;
        } catch (...) {
            return false;
        }
    }

    // gzip сжатие файла: src -> src+".gz"
    bool GzipCompressFile(const std::filesystem::path& src_path) {
        const std::string in_path  = src_path.string();
        const std::string out_path = in_path + ".gz";

        std::ifstream in(in_path, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "[Logger] Failed to open for gzip: " << in_path << "\n";
            return false;
        }

        gzFile out = gzopen(out_path.c_str(), "wb9");
        if (!out) {
            std::cerr << "[Logger] Failed to open gzip file: " << out_path << "\n";
            in.close();
            return false;
        }

        char buf[65536];
        while (in) {
            in.read(buf, sizeof(buf));
            std::streamsize got = in.gcount();
            if (got > 0) {
                if (gzwrite(out, buf, static_cast<unsigned int>(got)) != got) {
                    std::cerr << "[Logger] gzwrite failed for: " << out_path << "\n";
                    gzclose(out);
                    in.close();
                    return false;
                }
            }
        }

        gzclose(out);
        in.close();

        std::error_code ec;
        std::filesystem::remove(src_path, ec);
        if (ec) {
            std::cerr << "[Logger] Failed to remove original: " << in_path << "\n";
        }

        std::cout << "[Logger] Compressed: " << in_path << " -> " << out_path << "\n";
        return true;
    }

    void ProcessDirForCompression(const std::filesystem::path& dir, int days_to_keep) {
        using namespace std::chrono;

        auto now = system_clock::now();

        std::error_code ec;
        if (!std::filesystem::exists(dir, ec) || ec) return;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                if (ec) break;

                if (!entry.is_regular_file()) continue;

                const auto& p = entry.path();
                const std::string name = p.filename().string();

                // интересуют только .log
                if (p.extension() != ".log") continue;

                std::tm tm{};
                if (!ParseDateFromLogName(name, tm)) continue;

                std::time_t t = std::mktime(&tm);
                if (t == -1) continue;

                auto file_time = system_clock::from_time_t(t);
                auto diff = duration_cast<hours>(now - file_time).count() / 24;

                if (diff >= days_to_keep) {
                    GzipCompressFile(p);
                }
            }
        } catch (const std::exception& ex) {
            std::cerr << "[Logger] Exception in ProcessDirForCompression: " << ex.what() << "\n";
        }
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

    // Сжимаем старые логи (оставляем только текущий день)
    CompressOldLogs(1);
}

void CompressOldLogs(int days_to_keep) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_dir.empty()) return;

    // Обработка основной папки
    ProcessDirForCompression(std::filesystem::path(g_log_dir), days_to_keep);

    // Обработка каналов
    std::filesystem::path channels_dir = std::filesystem::path(g_log_dir) / "channels";
    std::error_code ec;
    if (std::filesystem::exists(channels_dir, ec)) {
        for (const auto& ch_entry : std::filesystem::directory_iterator(channels_dir, ec)) {
            if (ec) break;
            if (ch_entry.is_directory()) {
                ProcessDirForCompression(ch_entry.path(), days_to_keep);
            }
        }
    }
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

    const std::string file_name = MakeChannelLogFileName();
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
