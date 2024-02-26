/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#include "lnArduino.h"
#include "stdarg.h"
extern "C" void Logger_chars(int n, const char *data);

extern "C" void Logger_crash(const char *st)
{
}

extern "C" void Logger_C(const char *fmt, ...)
{
    static char buffer[128];

    va_list va;
    va_start(va, fmt);
    vsnprintf(buffer, 127, fmt, va);

    buffer[127] = 0;
    va_end(va);
    Logger_chars(strlen(buffer), buffer);
}
/**

*/
extern "C" void Logger_chars(int n, const char *data)
{
    if (!n)
        return; // 0 sized dma does not work...
}

/**
 *
 * @param fmt
 */
void Logger(const char *fmt...)
{
    static char buffer[128];

    if (fmt[0] == 0)
        return;

    va_list va;
    va_start(va, fmt);
    vsnprintf(buffer, 127, fmt, va);

    buffer[127] = 0;
    va_end(va);
}
/**
 *
 */
void LoggerInit()
{
    int debugUart = 0;
}
