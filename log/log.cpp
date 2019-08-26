#include "log.h"

#include <iostream>

#include "common/global.h"
#include "common/config/log_flags.h"

namespace comlog {

static const char* g_LogInfo[] = {
    "Info: ",
    "Warning: ",
    "Error: ",
    "Fatal: ",
    "Debug"
};  

Log::Log() {}

Log::~Log() {
    of_.close();
}

void Log::init(const char* log_file) {
    const char* connector_conf_path = "data/config/log.conf";
    char flag_file[128] = {0};
    snprintf(flag_file, sizeof(flag_file), "%s%s", g_prefix_res_path, connector_conf_path);
    google::SetCommandLineOption("flagfile", flag_file);

    of_.open(log_file, std::ios::app);
    if (!of_.is_open()) {
        return;
    }
    init_ = true;
}

void Log::write(LogType type, const char* log) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!init_) {
        init("default");
    }
    of_ << get_timestr()
        << " "
        << g_LogInfo[static_cast<int>(type)]
        << "."
        << " "
        << log
        << std::endl;

    if (FLAGS_log_stdout || type == LogType::DEBUG_LOG)
        std::cout << get_timestr()
                  << " "
                  << g_LogInfo[static_cast<int>(type)]
                  << "."
                  << " "
                  << log
                  << std::endl;
}

std::string Log::get_timestr() {
    time_t r_time;
    time(&r_time);
    struct tm t;
    localtime_r(&r_time, &t);
    char time_str[128] = {0};
    snprintf(time_str, sizeof(time_str), "%04d-%02d-%02d_%02d:%02d:%02d",
             t.tm_year + 1900,
             t.tm_mon + 1,
             t.tm_mday,
             t.tm_hour,
             t.tm_min,
             t.tm_sec);
    return std::string(time_str);
}

}
