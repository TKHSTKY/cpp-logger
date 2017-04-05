#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <pthread.h>
 #include <sys/syscall.h>
 #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "log_thread.h"

namespace log {

void Logger::InitConsoleLogger(FILE* output) {
    auto thread = Logger::Instance().Thread();
    if (output == stderr) {
        thread->AddWriter(std::unique_ptr<StderrLogWriter>(new StderrLogWriter()));
    } else {
        thread->AddWriter(std::unique_ptr<StdoutLogWriter>(new StdoutLogWriter()));
    }
}

void Logger::InitFileLogger(const char* filename) {
    auto thread = Logger::Instance().Thread();
    auto fileLogWriter = std::unique_ptr<FileLogWriter>(new FileLogWriter(filename));
    fileLogWriter->Init();
    thread->AddWriter(std::move(fileLogWriter));
}

Logger::Logger() : m_level(LogLevel::INFO) {
    m_thread = std::make_shared<LogThread>();
}

Logger::~Logger() {}

inline LogLevel Logger::Level() {
    return m_level;
}

inline void Logger::SetLevel(LogLevel level) {
    m_level = level;
}

inline bool Logger::IsEnabled(LogLevel level) {
    return m_level <= level;
}

inline std::shared_ptr<LogThread> Logger::Thread() {
    return m_thread;
}

#if defined(_WIN32) || defined(_WIN64)
static int vasprintf(char** strp, const char* fmt, va_list ap) {
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }
    size_t size = (size_t) (len + 1);
    *strp = (char*) malloc(size);
    if (*strp == nullptr) {
        return -1;
    }
    int result = vsnprintf(*strp, size, fmt, ap);
    if (result == -1) {
        free(*strp);
        return -1;
    }
    return result;
}

static int gettimeofday(struct timeval* tv, void* tz)
{
    const UINT64 epochFileTime = 116444736000000000ULL;
    FILETIME ft;
    ULARGE_INTEGER li;
    UINT64 t;

    if (tv == NULL) {
        return -1;
    }
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    t = (li.QuadPart - epochFileTime) / 10;
    tv->tv_sec = (long) (t / 1000000);
    tv->tv_usec = t % 1000000;
    return 0;
}
#endif // defined(_WIN32) || defined(_WIN64)

static uint64_t getCurrentThreadID();

void Logger::Log(LogLevel level, const char* file, uint32_t line, const char* fmt, ...) {
    if (!IsEnabled(level)) {
        return;
    }

    char* buf = nullptr;
    va_list arg;
    va_start(arg, fmt);
    if (vasprintf(&buf, fmt, arg) != -1) {
        LogMessage msg = {};
        msg.level = level;
        gettimeofday(&msg.timestamp, nullptr);
        msg.threadID = getCurrentThreadID();
        msg.file = file;
        msg.line = line;
        msg.content = buf;
        m_thread->Send(std::move(msg));
    } else {
        fprintf(stderr, "ERROR: logger: vasprintf");
    }
    va_end(arg);
}

static uint64_t getCurrentThreadID() {
#if defined(_WIN32) || defined(_WIN64)
    return (uint64_t) GetCurrentThreadId();
#elif __linux__
    return (uint64_t) syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    return (uint64_t) syscall(SYS_thread_selfid);
#else
    return (uint64_t) pthread_self();
#endif // defined(_WIN32) || defined(_WIN64)
}

} // namespace log
