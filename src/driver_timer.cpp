/**
 * @file driver_timer.cpp
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

#ifdef USE_CH32v3x_HW_IRQ_STACK
   #define LN_IRQ_FOS
#else
   #define LN_IRQ_FOS  __attribute__((interrupt))
#endif


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
/**  */
typedef struct
{
    volatile uint32_t CTLR;
    volatile uint32_t SR;
    volatile uint64_t CNT;
    volatile uint64_t CMP;
} SysTick_Type;

#define SysTick ((SysTick_Type *)0xE000F000)

/**
 */
volatile int sysTick = 0;
/**
 * @brief
 *
 */
extern "C" void SysTick_Handler(void)   __attribute__((used)) LN_IRQ_FOS;
extern "C" void SysTick_Handler(void) 
{
    sysTick++;
    SysTick->SR = 0;
}
/**
 * @brief
 *
 */
void setupSysTick()
{
    // setup sys tick
    SysTick->CTLR = 0;
    SysTick->SR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = LN_MCU_SPEED / 1000; // 1ms tick
    SysTick->CTLR = 0xf;
}
/**
 * @brief
 *
 * @param us
 */
void delayMicroseconds(int us)
{
    int ms = 1 + (us / 1000);
    xDelay(ms);
}
/**
 * @brief
 *
 * @return uint32_t
 */
uint32_t lnGetMs()
{
    return sysTick;
}
/**
 * @brief
 *
 * @param a
 */
void xDelay(unsigned int a)
{
    int tail = sysTick + a;
    while (sysTick < tail)
    {
        __asm__("nop");
    }
}
/**
 * @brief
 *
 * @param delay
 */
void lnDelay(unsigned int delay)
{
    xDelay(delay);
}

// EOF