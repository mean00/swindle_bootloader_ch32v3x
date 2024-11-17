/*
 *  (C) 2022/2023 MEAN00 fixounet@free.fr
 *  See license file
 *  PFIC is configured with 2 nested level, 1 bit for preemption
 *  That means interrupt priority between 0..7 ignoring preemption
 *
 */
#include "ch32v30x_isr_helper.h"
#include "lnArduino.h"
#include "lnIRQ.h"
#include "lnIRQ_riscv_priv_ch32v3x.h"
#include "lnRCU.h"

#ifdef USE_CH32v3x_HW_IRQ_STACK
#define HANDLER_DESC(x)                                                                                                \
    extern "C" void x();                                                                                               \
    extern "C" void x##_relay() LN_INTERRUPT_TYPE;
#define HANDLER_DESC_C(y)                                                                                              \
    extern "C" void y();                                                                                               \
    extern "C" void y##_relay() LN_INTERRUPT_TYPE;
extern "C" void unsupported_relay();
#define LOCAL_LN_INTERRUPT_TYPE
#define WCH_HW_STACK CH32_SYSCR_HWSTKEN
#else
#define LOCAL_LN_INTERRUPT_TYPE LN_INTERRUPT_TYPE
#define HANDLER_DESC(x) extern "C" void x() LOCAL_LN_INTERRUPT_TYPE;
#define HANDLER_DESC_C(y) extern "C" void y() LOCAL_LN_INTERRUPT_TYPE;
#define WCH_HW_STACK 0
#endif
#define HANDLER_DESC_RAW(y) extern "C" void y() LOCAL_LN_INTERRUPT_TYPE;

#include "local_interrupt_table.h"
/**
 *
 */

LIST_OF_HANDLERS

/**
 * @brief
 *
 */
extern "C" void __attribute__((noinline)) unsupported()
{
    deadEnd(11);
}
/**

//---------------------------------------------------------------------
//---------------------------------------------------------------------
/*- Create vector table -*/
#undef INTERRUPT_DESC
//--
#define INTERRUPT_DESC_RAW(y) (uint32_t)y
#ifdef USE_CH32v3x_HW_IRQ_STACK
#define INTERRUPT_DESC(y) (uint32_t)y##_relay
#else
#define INTERRUPT_DESC(y) (uint32_t)y
#endif
//--
#define UNSUPPORTED_NO(y) (uint32_t)unsupported_##y

#define VECTOR_TABLE __attribute__((section(".vector_table")))
//--
extern VECTOR_TABLE const uint32_t vecTable[] __attribute__((aligned(32)));
VECTOR_TABLE const uint32_t vecTable[] __attribute__((aligned(32))) = {LIST_OF_INTERRUPTS};
//--
#define SIZE_OF_VEC_TABLE sizeof(vecTable) / sizeof(uint32_t)
extern const uint32_t size_of_vec_table = SIZE_OF_VEC_TABLE;

uint8_t vec_revert_table[SIZE_OF_VEC_TABLE];

#undef INTERRUPT_DESC

#define WEAK_INTERRUPT(y)                                                                                              \
    extern "C" void __attribute__((weak)) y()                                                                          \
    {                                                                                                                  \
        xAssert(0);                                                                                                    \
    }

WEAK_INTERRUPT(USB_WAKEUP_IRQHandler)
WEAK_INTERRUPT(OTG_FS_IRQHandler)

#define RELAY_FUNC(x)                                                                                                  \
    ISR_CODE extern "C" void __attribute__((naked)) x##_relay()                                                        \
    {                                                                                                                  \
        __asm__("jal " #x "\n"                                                                                         \
                "mret");                                                                                               \
    }
#define RELAY_DMA(d, c) RELAY_FUNC(DMA##d##_Channel##c##_IRQHandler)

//---- Relay func
#ifdef USE_CH32v3x_HW_IRQ_STACK
RELAY_FUNC(USART0_IRQHandler)
RELAY_FUNC(SysTick_Handler)
RELAY_FUNC(OTG_FS_IRQHandler)
RELAY_FUNC(unsupported)

#endif
// EOF
