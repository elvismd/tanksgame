#include "logger.h"

#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif

Logger* _logger;

void set_logger(Logger* logger)
{
    _logger = logger;  
    _logger->console_output = true; 
}

internal void _log(const char* fmt, Log_Type type, va_list args)
{
    int len =  vsnprintf(NULL, 0, fmt, args);

    if((len) >= array_count(_logger->logs))
        return; // TODO: clear old logs

    if(len > 0)
        vsnprintf(_logger->logs, array_count(_logger->logs), fmt, args);
    else 
        snprintf(_logger->logs, array_count(_logger->logs), fmt);

    char str_type[1024];
    switch(type)
    {
        case _INFO: snprintf(str_type, 1024, "[Info]"); break;
        case _WARNING: snprintf(str_type, 1024, "[Warning]"); break;
        case _ERROR: snprintf(str_type, 1024, "[Error]"); break;
    }

    printf("%s %s\n", str_type, _logger->logs);
}

void log_info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    _log(fmt, _INFO, args);
    
    va_end(args);
}

void log_warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    _log(fmt, _WARNING, args);
    
    va_end(args);
}

void log_error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    _log(fmt, _ERROR, args);

    perror("Following error ocurred: ");

    va_end(args);

    exit(12);
}