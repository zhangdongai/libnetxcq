#pragma once

#include <stdio.h>
#include <fstream>
#include <mutex>
#include <time.h>

#include "common/macros.h"

namespace comlog {

enum class LogType {
    INFO_LOG = 0,
    WARN_LOG = 1,
    ERROR_LOG = 2,
    FATAL_LOG = 3,
    DEBUG_LOG = 4,
};

static const bool std_out = true;

class Log {
public:
    ~Log();

    void write(LogType type, const char* log);
    void init(const char* log_file);

private:
    std::string get_timestr();

    std::ofstream of_;
    bool init_ = false;
    std::mutex mutex_;

    SINGLETON_DECLARE(Log);
};

}  // namespace comlog

#define DEBUG_LOG(fmt, ...)                                 \
    {char log_data[1024] = {0};                              \
    snprintf(log_data, sizeof(log_data), fmt, ##__VA_ARGS__); \
    comlog::Log::instance()->write(                         \
        comlog::LogType::DEBUG_LOG, log_data);               \
    }
#define INFO_LOG(fmt, ...)                             \
    {char log_data[1024] = {0};                              \
    snprintf(log_data, sizeof(log_data), fmt, ##__VA_ARGS__); \
    comlog::Log::instance()->write(                         \
        comlog::LogType::INFO_LOG, log_data);              \
    }
#define WARN_LOG(fmt, ...)                             \
    {char log_data[1024] = {0};                              \
    snprintf(log_data, sizeof(log_data), fmt, ##__VA_ARGS__); \
    comlog::Log::instance()->write(                         \
        comlog::LogType::WARN_LOG, log_data);              \
    }
#define ERROR_LOG(fmt, ...)                            \
    {char log_data[1024] = {0};                              \
    snprintf(log_data, sizeof(log_data), fmt, ##__VA_ARGS__); \
    comlog::Log::instance()->write(                         \
        comlog::LogType::ERROR_LOG, log_data);            \
    }
#define FATAL_LOG(fmt, ...)                            \
    {char log_data[1024] = {0};                              \
    snprintf(log_data, sizeof(log_data), fmt, ##__VA_ARGS__); \
    comlog::Log::instance()->write(                         \
        comlog::FATAL_LOG, log_data);                      \
    }
