/**
 * @file stubs_lowlevel.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

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

STUBME(noInterrupts)
STUBME(interrupts)
STUBME(DisableIrqs)
STUBME(EnableIrqs)

STUBME_C(SW_Handler)
STUBME_C(USART0_IRQHandler)

STUBME(systemReset)

/**
 * @brief
 *
 */
void uartInit()
{
}
void uartPutChar(char c)
{
}
extern "C" void uartSend_C()
{
}

volatile uint32_t sysTick;
/**
 * @brief
 *
 */
void setupSysTick()
{
    sysTick = 0;
}
/**
 * @brief
 *
 */
extern "C" void SysTick_Handler(void)
{
    sysTick++;
}
/**
 * @brief
 *
 * @param a
 */
void lnDelay(unsigned int a)
{
    uint32_t limit = sysTick + a + 1;
    while (1)
    {
        __asm__("nop");
        if (sysTick > limit)
            return;
    }
}
/**
 * @brief
 *
 */
void __attribute__((noreturn)) do_assert(const char *a)
{
    __asm__("ebreak");
    while (1)
        __asm__("nop");
}
/**
 * @brief
 *
 * @param a
 * @param b
 */
void dmaIrqHandler(int a, int b)
{
    xAssert(0);
}
// EOF