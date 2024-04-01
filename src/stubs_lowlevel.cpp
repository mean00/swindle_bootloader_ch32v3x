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
/**
 * @brief 
 * 
 */
char _ctype_b[10];
extern __attribute__((section(".initial_heap"))) uint8_t ucHeap[8*1024];
/**
 * @brief 
 * 
 */
extern "C" void vPortEnterCritical()
{
    ucHeap[0]=0; // mark as used
    deadEnd(0);
}
/**
 * @brief 
 * 
 */
extern "C" void vPortExitCritical()
{
    deadEnd(0);
}
/**
 * @brief 
 * 
 * @param periph 
 */
void resetMe(const Peripherals periph)
{
    lnPeripherals::reset(periph);
    lnPeripherals::enable(periph);
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

STUBME_C(SW_Handler)
STUBME_C(USART0_IRQHandler)
// EOF