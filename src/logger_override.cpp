//
#include "lnArduino.h"
#include "lnSerial.h"
#include "stdarg.h"
extern "C" void Logger_crash(const char *st)
{
}

extern "C" int Logger_C(const char *fmt, ...)
{
    static char buffer[128];

    va_list va;
    va_start(va, fmt);
    vsnprintf(buffer, 127, fmt, va);

    buffer[127] = 0;
    va_end(va);
    Logger_chars(strlen(buffer), buffer);
    return 0;
}
/**

*/
extern "C" void Logger_chars(int n, const char *data)
{
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

    Logger_chars(strlen(buffer), buffer);
}
/**
 *
 */
void LoggerInit()
{
}
