/**
 * @file driver_timer.cpp
 * @brief
 * @version 0.1
 * @date 2024-02-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "embedded_printf/printf.h"
#include "lnArduino.h"
#include "lnPeripheral_priv.h"
#include "lnSerial.h"
#include "lnSerial_priv.h"

#define STUBME(x)                                                                                                      \
    void x()                                                                                                           \
    {                                                                                                                  \
        xAssert(0);                                                                                                    \
    }
#define STUBME_C(x)                                                                                                    \
    extern "C" void x()                                                                                                \
    {                                                                                                                  \
        xAssert(0);                                                                                                    \
    }

STUBME_C(USART0_IRQHandler)

#define usart0 ((LN_USART_Registers *)LN_USART0_ADR)
/**
 * @brief
 *
 */
void uartInit()
{
    LN_USART_Registers *d = usart0;
    lnPinMode(PA9, lnALTERNATE_PP);
    // Disable RX & TX
    d->CTL0 &= ~LN_USART_CTL0_KEEP_MASK; // default is good enough
    // speed setSpeed(115200);
    int speed = 115200;
    int freq;
    freq = lnPeripherals::getClock(pUART0);
    // compute closest divider
    int divider = (freq + (speed / 2)) / speed;
    // Fixed 4 buts decimal, just ignore since we oversample by 16
    d->CTL0 &= ~LN_USART_CTL0_UEN;
    d->BAUD = divider; // change speed when usart is off
    d->CTL0 |= LN_USART_CTL0_TEN;
    d->CTL0 |= LN_USART_CTL0_UEN;
}
/**
 * @brief
 *
 * @param c
 */
void uartPutChar(const char c)
{
    LN_USART_Registers *d = usart0;
    while (!(d->STAT & LN_USART_STAT_TBE))
    {
        __asm__("nop");
    }
    d->DATA = c;
}
/**
 * @brief
 *
 * @param c
 */
void uartSend(const char *c)
{
    LN_USART_Registers *d = usart0;
    while (*c)
    {

        while (!(d->STAT & LN_USART_STAT_TBE))
        {
            __asm__("nop");
        }
        d->DATA = *c;
        c++;
    }
}
/**
 * @brief
 *
 */
extern "C" void uartSend_C(const char *c)
{
    LN_USART_Registers *d = usart0;
    while (*c)
    {

        while (!(d->STAT & LN_USART_STAT_TBE))
        {
            __asm__("nop");
        }
        d->DATA = *c;
        c++;
    }
}

// EOF