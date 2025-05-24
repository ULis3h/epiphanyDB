#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "log.hpp"


namespace epiphany
{

Log *g_log = NULL;

Log::Log(const std::string &log_name, const LogLevel log_level, 
         const LogLevel console_level)
        : m_log_name_(log_name), m_log_level_(log_level), 
          m_console_level_(console_level)
{

    m_log_date_.m_year_ = m_log_date_.m_mon_ = m_log_date_.m_day_ = -1;

}

Log::~Log(void)
{ }

const char* Log::prefix_msg(LogLevel level)
{
    if (LogLevel_Panic <= level && level < LogLevel_Last)
    {
        return LogPrefixMap[level];
    } 
    static const char* empty_prefix = "";
    return empty_prefix;
}

bool
Log::check_output(const LogLevel level, const char* module)
{
    if (LogLevel_Last > level && level <= m_console_level_) {
        return true;
    }
    else if (LogLevel_Last > level && level < m_log_level_) {
        return true;
    }
    else { ; }   
    return false;
}

int 
Log::output(const LogLevel level, const char* module, const char *prefix, 
            const char* f, ...)
{
    va_list args;
    char msg[1024];

    va_start(args, f);
    vsnprintf(msg, sizeof(msg), f, args);
    va_end(args);

    if (LogLevel_Panic <= level && level <= m_console_level_) {
        std::cout << prefix << msg << std::endl;
    }
    return 0;
}

int 
LoggerFactory::init(const std::string &log_file, Log **logger, 
                    LogLevel log_level, LogLevel console_level)
{
    Log* log = new Log(log_file, log_level, console_level);
    if (log == NULL)
    {
        std::cout << "Error: Fail to construct a log object!" << std::endl;
        return -1;
    }
    *logger = log;
    return 0;
}

int 
LoggerFactory::init_default(const std::string &log_file,LogLevel log_level, 
                            LogLevel console_level)
{
    if (g_log != NULL)
    {
        LogInfo("Default logger has been initialized");
        return 0;
    }
    return init(log_file, &g_log, log_level, console_level);
}
}