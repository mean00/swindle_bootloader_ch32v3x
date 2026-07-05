/**
 * @file    stage2_entry.cpp
 * @brief   Stage-2 bootloader entry point and startup sequence.
 *
 * @details This file implements the stage-2 C runtime initialisation and
 *          entry point.  It is the first code executed when stage-1 chains
 *          to RAM at 0x20000000.
 *
 *          Responsibilities:
 *            1. Set the stack pointer to the top of RAM.
 *            2. Copy the .data section from its LMA (in flash, after .rodata)
 *               to its VMA (0x20008000+ in RAM).
 *            3. Zero the .bss section (including .sbss).
 *            4. Initialise the system clock (144 MHz).
 *            5. Run global constructors via @c __libc_init_array.
 *            6. Initialise interrupt controller and SysTick.
 *            7. Call @c stage2_main() which runs the DFU loop.
 *
 * @note    Stage-2 is linked at 0x20000000 and does not have a start.S —
 *          all low-level init is performed here in C with inline assembly.
 *
 * @ingroup stage2
 */

#include "stdint.h"
extern void lnInitSystemClock();
extern void stage2_main();
extern void stage2_irq_init(void);
extern const char __data_begin__, __data_end__;
extern const char __bss_begin__, __bss_end__;
extern const char __data_lma__;
extern "C" void __libc_init_array();

/**
 * @brief  C++ portion of stage-2 initialisation.
 */
extern "C" void stage2_cpp_entry(void)
{
    // Copy .data from LMA (flash, after .rodata) to VMA (0x20008000+)
    __asm volatile("  mv t0, %0\n"
                   "  mv t1, %1\n"
                   "  mv t2, %2\n"
                   "lp0:          \n"
                   "  bge t1, t2, end_lp0\n"
                   "  lw t3, 0(t0)\n"
                   "  sw t3, 0(t1)\n"
                   "  addi t0, t0, 4\n"
                   "  addi t1, t1, 4\n"
                   "  j lp0\n"
                   "end_lp0:\n"

                   // Zero .bss (includes .sbss*)
                   "  mv t0, %3\n"
                   "  mv t1, %4\n"
                   "lp1:          \n"
                   "  bge t0, t1, end_lp1\n"
                   "  sw zero, 0(t0)\n"
                   "  addi t0, t0, 4\n"
                   "  j lp1\n"
                   "end_lp1:\n"
                   :
                   : "r"((uint32_t *)&__data_lma__),
                     "r"((uint32_t *)&__data_begin__),
                     "r"((uint32_t *)&__data_end__),
                     "r"((uint32_t *)&__bss_begin__),
                     "r"((uint32_t *)&__bss_end__)
                   : "t0", "t1", "t2", "t3", "memory");

    // Clock init before any constructor that might read SystemCoreClock
    lnInitSystemClock();

    // Global constructors
    __libc_init_array();

    // Interrupt + timer setup
    stage2_irq_init();

    // Main DFU loop
    stage2_main();
    
    while(1);
}

/**
 * @brief  Stage-2 entry point — called directly by stage-1 via chain_to().
 *
 * @note This function is placed in the @c .text.entry section and never
 *       returns. It is 'naked' to prevent the compiler from generating any
 *       prologue/epilogue (__riscv_save) that uses the stack before we are ready.
 */
extern "C" __attribute__((section(".text.entry"), naked)) void stage2_entry(void)
{
    __asm volatile(
        "  li sp, 0x2000FFF0\n"
        "  tail stage2_cpp_entry\n"
    );
}
