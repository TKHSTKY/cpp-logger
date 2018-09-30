#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
 #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
 #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif // defined(_WIN32) || defined(_WIN64)

#define LOG_TRACE(fmt, ...) logger::Logger::Instance().Log(logger::LogLevel_TRACE, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) logger::Logger::Instance().Log(logger::LogLevel_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  logger::Logger::Instance().Log(logger::LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger::Logger::Instance().Log(logger::LogLevel_WARN , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger::Logger::Instance().Log(logger::LogLevel_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) logger::Logger::Instance().Log(logger::LogLevel_FATAL, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)

namespace logger {

enum LogLevel : uint8_t {
    LogLevel_TRACE,
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
    LogLevel_FATAL,
};

class LogThread;

class Logger final {
public:
    static Logger& Instance() {
        static Logger s_instance;
        return s_instance;
    }

    static bool InitConsoleLogger(FILE* output = stdout);
    static bool InitFileLogger(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles);

    static LogLevel Level() {
        return Logger::Instance().level();
    }
    static void SetLevel(LogLevel level) {
        Logger::Instance().setLevel(level);
    }
    static bool IsEnabled(LogLevel level) {
        return Logger::Instance().isEnabled(level);
    }
    static void AutoFlush(long interval) {}
    static void Flush() {}

    std::shared_ptr<LogThread> Thread();
    void Log(LogLevel level, const char* file, uint32_t line, const char* fmt, ...);

private:
    LogLevel m_level;
    std::shared_ptr<LogThread> m_thread;

    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel level();
    void setLevel(LogLevel level);
    bool isEnabled(LogLevel level);
};

} // namespace logger
