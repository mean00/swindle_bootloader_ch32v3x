/**
 * \brief Small(ish) replacement for printf
 *
 */

#include "lnArduino.h"
#include "stdarg.h"
void uartSend(const char *c);
/**
 * @brief
 *
 * @param c
 * @param hex
 */
void printCHex(const char *c, uint32_t hex)
{

}
extern "C" int Logger_C(const char *c, ...)
{
      uartSend(c);
      return 0;
}
/**
 * @brief
 *
 */
extern "C" void Logger_crash(const char *st)
{
      uartSend("** CRASH **\n");
      uartSend(st);
}

/**
 * @brief
 *
 * @param c
 */
void printC(const char *c)
{
        uartSend(c);
}
//
