#pragma once

#include <stdarg.h> 
#include <stdlib.h>
 #include <stdio.h>

#include "agnostic.h"

enum Log_Type
{
    _INFO,
    _WARNING,
    _ERROR,
};

struct Logger
{
    char logs[3000000];
    u32 size;
    
    b32 console_output;
    b32 win32_output;
};

void set_logger(Logger* logger);
void log_info(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_error(const char* fmt, ...);