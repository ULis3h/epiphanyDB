#ifndef COMMON_LOG_HPP_
#define COMMON_LOG_HPP_
#include <string>

namespace epiphany
{

/// @brief 日志等级定义.
enum LogLevel
{
    LogLevel_Panic = 0,
    LogLevel_Error = 1,
    LogLevel_Warn = 2,
    LogLevel_Info = 3,
    LogLevel_Debug = 4,
    LogLevel_Trace = 5,
    LogLevel_Last,
};

static const char* LogPrefixMap[LogLevel_Last] = 
{
    "Panic",
    "Error",
    "Warn",
    "Info",
    "Debug",
    "Trace",
};

class Log
{
public:
    Log(const std::string &log_name, const LogLevel log_level = LogLevel_Info, 
        const LogLevel console_level = LogLevel_Warn);

    ~Log(void);    

private:
    struct LogDate
    {
        int m_year_;
        int m_mon_;
        int m_day_;
    };

public:
    static int init(const std::string &log_file);

    int output(const LogLevel level, const char* module, const char *prefix, const char* f, ...);

    const char* prefix_msg(const LogLevel level);

    bool check_output(const LogLevel log_level, const char* module);

private:
    std::string m_log_name_;
    LogLevel m_log_level_;
    LogLevel m_console_level_;

    LogDate m_log_date_;
};

class LoggerFactory
{
public:
    LoggerFactory();
    virtual ~LoggerFactory();

    /// @brief 
    /// @param log_file 
    /// @param logger 
    /// @param log_level 
    /// @param console_level 
    /// @return 
    static int init(const std::string &log_file, Log **logger, 
                    LogLevel log_level = LogLevel_Info,
                    LogLevel console_level = LogLevel_Warn);

    static int init_default(const std::string &log_file, 
                            LogLevel log_level = LogLevel_Info,
                            LogLevel console_level = LogLevel_Warn);                
};

extern Log *g_log;

#define LogHeadSize 128

#define LogHead(prefix, level)                                           \
if (epiphany::g_log)                                                          \
{                                                                        \
    char sz_head[LogHeadSize] = {0};                                     \
    snprintf(prefix, sizeof(prefix),                                     \
             "[%s \033[32m%s\033[0m %s@%s:%u]:  ", sz_head,              \
             epiphany::g_log->prefix_msg(level), __FUNCTION__, __FILE_NAME__, \
             (int)__LINE__);                                             \
}

#define LogOutPut(level, fmt, ...)                                       \
do                                                                       \
{                                                                        \
    using namespace epiphany;                                                 \
    if (g_log && g_log->check_output(level, __FILE_NAME__))              \
    {                                                                    \
        char prefix[1024] = {0};                                         \
        LogHead(prefix, level);                                          \
        g_log->output(level, __FILE_NAME__, prefix, fmt, ##__VA_ARGS__); \
    }                                                                    \
} while (0)

}

//TODO: Painc/Error/Warn/Debug/Trace.
#define LogInfo(fmt, ...) LogOutPut(epiphany::LogLevel_Info, fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...)
#define LogErr(fmt, ...)
#define LogDbg(fmt, ...)
#define LogPanic(fmt, ...)
#define LogTrace(fmt, ...)

#endif //! COMMON_LOG_HPP_