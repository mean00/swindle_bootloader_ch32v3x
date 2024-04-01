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

static  LN_USART_Registers *usart0 =((LN_USART_Registers *)LN_USART0_ADR);
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
 */
void uartDeinit()
{
    LN_USART_Registers *d = usart0;
    lnPinMode(PA9, lnALTERNATE_PP);
    // Disable RX & TX
    d->CTL0 &= ~LN_USART_CTL0_KEEP_MASK; // default is good enough
    d->CTL0 &= ~LN_USART_CTL0_UEN;    
    d->CTL0 &= ~LN_USART_CTL0_TEN;    
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
        uartPutChar(*c);
        c++;
    }
}
/**
 * @brief
 *
 */
extern "C" void uartSend_C(const char *c)
{
   uartSend(c);
}

// EOF