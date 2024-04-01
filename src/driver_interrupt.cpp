/**
 * @file driver_interrupt.cpp
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

#include "lnRCU_priv.h"

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
/**
 * @brief
 *
 */
void EnableIrqs()
{
      __asm volatile("csrs mstatus, %0" :: "r"(0x88) );
}
/**
 * @brief
 *
 */
void DisableIrqs()
{
      __asm volatile("csrc mstatus, %0" :: "r"(0x88) );
}
/**
 * @brief
 *
 */
void noInterrupts()
{
    DisableIrqs();
}
/**
 * @brief
 *
 */
void interrupts()
{
    EnableIrqs();
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
 */
void LN_INTERRUPT_TYPE xxxOTG_FS_IRQHandler()
{
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
/**
 * @brief
 *
 */
void systemReset()
{
    volatile uint32_t *pfic_sctlr = (volatile uint32_t *)0xE000ED10;
    *pfic_sctlr = (1 << 31);
    while (1)
    {
        __asm__("nop");
    }
}
/**
 * @brief 
 * 
 */
extern LN_RCU *arcu;
extern "C" void clockDefault()
{
    uint32_t clks =  arcu->CFG0;
   // now switch system clock to IRC8
    clks &= ~(LN_RCU_CFG0_SYSCLOCK_MASK);
    clks |= LN_RCU_CFG0_SYSCLOCK_IRC8; // switch back to IRC8 as syslock
    arcu->CFG0 = clks;
    arcu->CTL &= ~LN_RCU_CTL_PLLEN; // stop PLL
}

// EOF