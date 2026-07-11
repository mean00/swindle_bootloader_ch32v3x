/**
 * @file    stage2_irq.cpp
 * @brief   Interrupt vector and SysTick setup for the CH32V3x stage-2 bootloader.
 *
 * @details Implements the WCH vectored interrupt mode initialisation, the
 *          default interrupt handler, and the SysTick tick counter query.
 *          Separated from dfu.cpp to keep IRQ setup logic self-contained.
 *
 * @ingroup stage2
 */

#include "esprit.h"
#include "registers.h"

// ========== SysTick (system timer) interrupt setup ==========
// Stage 2 uses polled USB, but enables SysTick for timing.

// --- PFIC (interrupt controller) register layout ---
#define PFIC_BASE 0xE000E000
#define PFIC_CFGR (*(volatile uint32_t *)(PFIC_BASE + 0x48))
#define PFIC_IPRIOIR ((volatile uint32_t *)(PFIC_BASE + 0x400))

// --- WCH-specific CSRs ---
#define CSR_INTMASK 0xBC0  // CPU-level interrupt mask
#define CSR_INTSYSCR 0x804 // Interrupt system controller

// CSR 0xBC0 — CPU-level interrupt mask (WCH-specific).
// Bits (from QingKe V4F documentation):
//   [0] = MSIE  (machine software interrupt)
//   [1] = MTIE  (machine timer interrupt) – keep CLEAR; the core timer
//         mtimecmp defaults to 0, so setting this causes an immediate
//         spurious tick that vectors through vec_table[7].
//   [2] = MEIE  (machine external interrupt - PFIC)
//   [3-4] = reserved / other
#define CSR_BC0_MSIE (1 << 0)
#define CSR_BC0_MTIE (1 << 1) // deliberately omitted
#define CSR_BC0_MEIE (1 << 2)
#define CSR_BC0_OTHER ((1 << 3) | (1 << 4))
#define INTMASK_VALUE (CSR_BC0_MSIE | CSR_BC0_MEIE | CSR_BC0_OTHER) // 0x1D

// INTSYSCR (CSR 0x804) bits.
// NOTE: HWSTKEN (hardware stack) must NOT be set when ISRs use
// __attribute__((interrupt)) — the hardware would double-save registers.
// When HWSTKEN=0 the CPU uses software stack mode and each ISR
// saves/restores its own context via the interrupt attribute.
#define INTSYSCR_HWSTKEN (1 << 0)   // HW stack enable (conflicts with __attribute__((interrupt)))
#define INTSYSCR_INESTEN (1 << 1)   // Nested interrupt enable
#define INTSYSCR_MPTCFG_2 (1 << 2)  // 2-level priority threshold
#define INTSYSCR_HWSTKOVEN (1 << 4) // Hardware stack overflow enable
#define INTSYSCR_VALUE (INTSYSCR_INESTEN | INTSYSCR_MPTCFG_2 | INTSYSCR_HWSTKOVEN)

// --- mie (Machine Interrupt Enable, CSR 0x304) bits ---
// NOTE: On CH32V3x the system timer (0xE000F000) is a PFIC external
// interrupt (#12), NOT the core RISC-V timer. Only set MEIE (bit 11).
// MTIE (bit 7) must stay 0 or the core mtime (default mtimecmp=0) could
// fire immediately and vector through garbage entries in our table.
#define MIE_MEIE (1 << 11) // Machine External Interrupt
#define MIE_EXT_TIMER (MIE_MEIE)

// --- mtvec mode encoding ---
#define MTVEC_MODE_MASK 3
#define MTVEC_WCH_VECTORED 3 // WCH vectored (PFIC)

// --- PFIC configuration constants ---
#define PFIC_CFGR_VECTORED (1 << 0) // Vectored mode enable
#define PFIC_CFGR_RESETSYS (1 << 7) // System reset bit
#define PFIC_IPRIOIR_COUNT 64
#define PFIC_DEFAULT_PRIO 5
#define PFIC_PRIO(x) ((x) << 4) // Priority field in IPRIOIR entry
#define PFIC_VECTOR_SYSTICK 12

// Vector table for WCH vectored interrupt mode
#define VEC_TABLE_SIZE 256
#define VEC_TABLE_ALIGN 64
__attribute__((aligned(VEC_TABLE_ALIGN))) static uint32_t vec_table[VEC_TABLE_SIZE] = {0};

extern void systick_isr(void);
extern uint32_t getTickCount();

/**
 * @brief  Default interrupt handler — catches any unexpected interrupt.
 * @note   Captures mie/mstatus CSRs for debugging, then returns (mret).
 */
__attribute__((interrupt)) static void default_vector(void)
{ xAssert(0); }

/**
 * @brief  Initialise vectored interrupt mode and SysTick for 1ms periodic ticks.
 *
 * USB remains polled — only SysTick uses the interrupt system.
 * Called once after clock init, before USB init.
 *
 * @note   CSR_INTMASK enables WCH interrupt delivery at the CPU level
 *         (normally done in start.S, which stage2 does not run).
 *         CSR_INTSYSCR enables hardware stacking and nested interrupts.
 *         PFIC IPRIOIR must be given a default priority for all entries.
 */
void stage2_irq_init(void)
{
    extern uint32_t SystemCoreClock;

    // 1. Set up the vector table — fill all 256 entries with a default
    //    handler (catches any spurious interrupt), then override SysTick.
    for (int i = 0; i < VEC_TABLE_SIZE; i++)
        vec_table[i] = (uint32_t)&default_vector;
    vec_table[PFIC_VECTOR_SYSTICK] = (uint32_t)&systick_isr;

    // 2. Enable CPU-level interrupt delivery (normally done in start.S)
    __asm volatile("li t0, %0\n"
                   "csrw %1, t0"
                   :
                   : "i"(INTMASK_VALUE), "i"(CSR_INTMASK));

    // 3. Set mie to accept external + timer interrupts
    __asm volatile("li t0, %0\n"
                   "csrw mie, t0"
                   :
                   : "i"(MIE_EXT_TIMER));

    // 4. Configure WCH interrupt system controller (INTSYSCR)
    __asm volatile("li t0, %0\n"
                   "csrw %1, t0"
                   :
                   : "i"(INTSYSCR_VALUE), "i"(CSR_INTSYSCR));

    // 5. Set all PFIC priority entries to a default value
    //    Interrupts with priority below the threshold (default 0) are blocked.
    uint32_t prio32 = PFIC_PRIO(PFIC_DEFAULT_PRIO) * 0x01010101;
    for (int i = 0; i < PFIC_IPRIOIR_COUNT; i++)
        PFIC_IPRIOIR[i] = prio32;

    // 6. Enable PFIC vectored mode
    PFIC_CFGR |= PFIC_CFGR_VECTORED;

    // 7. Point mtvec to the vector table (WCH vectored mode)
    __asm volatile("csrw mtvec, %0" : : "r"(((uint32_t)vec_table) | MTVEC_WCH_VECTORED));
}

/**
 * @brief  Return the 1-ms tick count for timeout calculations.
 * @return uint32_t  The current tick count from SysTick.
 */
uint32_t stage2_get_ticks(void)
{ return getTickCount(); }

/**
 * @brief  Trigger a system reset via the PFIC (Programmable Fast Interrupt Controller).
 */
void system_reset(void)
{
    PFIC_CFGR = PFIC_CFGR_RESETSYS | (0xbeefU << 16U);
    while (1)
    {
        asm("nop");
    } // wait for reset
}
