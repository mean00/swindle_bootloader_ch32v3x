/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */
#if 0
#include "lnArduino.h"
#include "stdarg.h"

#define PRINT_BUFFER_SIZE 128
static char buffer[PRINT_BUFFER_SIZE + 1];

extern "C" void Logger_chars(int n, const char *data);
extern "C" void uartSend_C(const char *c);
/**
 * @brief
 *
 */
extern "C" void Logger_crash(const char *st)
{
}
/**
 * @brief
 *
 */
extern "C" void Logger_C(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsnprintf(buffer, PRINT_BUFFER_SIZE, fmt, va);

    buffer[PRINT_BUFFER_SIZE] = 0;
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

    if (fmt[0] == 0)
        return;

    va_list va;
    va_start(va, fmt);
    vsnprintf(buffer, PRINT_BUFFER_SIZE, fmt, va);

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

/**
 * @brief
 *
 * @param c
 * @param hex
 */
void printCHex(const char *c, uint32_t hex)
{
    snprintf_(buffer, PRINT_BUFFER_SIZE, "%s 0x:%x\n", c, hex);
    buffer[PRINT_BUFFER_SIZE] = 0;
    uartSend_C(buffer);
}
/**
 * @brief
 *
 * @param c
 */
void printC(const char *c)
{
    uartSend_C(c);
}
#endif
// EOF
