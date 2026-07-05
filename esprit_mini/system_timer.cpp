#include "esprit.h"

// Machine timer (mtime) at 0xE000F000
typedef struct
{
    uint32_t CTLR;   // 0x00
    uint32_t SR;     // 0x04
    uint32_t CTN_LO; // 0x08
    uint32_t CTN_HI; // 0x0C
    uint32_t CMP_LO; // 0x10
    uint32_t CMP_HI; // 0x14
} lnStkx;

// CTLR bit definitions
#define SYSTICK_STEN (1 << 0)      // System Timer Enable
#define SYSTICK_STIE (1 << 1)      // System Timer Interrupt Enable
#define SYSTICK_STCLK (1 << 2)     // if 0, use hclk/8
#define SYSTICK_STRE (1 << 3)      // autoreload
#define SYSTICK_MODE_UP (0 << 4)   // count up
#define SYSTICK_MODE_DOWN (1 << 4) // count down
#define SYSTICK_INIT (1 << 5)      // reset counter on start

typedef volatile lnStkx lnStk;
lnStk *stk = (lnStk *)LN_STK_ADR; // E000F000

static uint64_t tickPerUs = 1;
// SysTick 1ms tick counter
static volatile uint32_t sys_tick_count = 0;

/**
 * @brief SysTick interrupt handler — called every 1ms via vectored interrupt.
 *        Uses STRE-autoreload mode; the timer resets CNT to 0 on match.
 *        Only clears the status flag and increments the tick counter.
 */
__attribute__((interrupt)) void systick_isr(void)
{
    // Clear status flag (CNT was already reset to 0 by STRE autoreload)
    stk->SR = 0;
    sys_tick_count++;
}

/**
 * @brief Return the number of milliseconds since SysTick was started.
 * @return 32-bit millisecond counter (wraps after ~49 days).
 */
uint32_t getTickCount()
{ return sys_tick_count; }

/**
 * @brief Initialise the µs-conversion factor.
 *        Must be called after SystemCoreClock is set (but before lnGetUs).
 */
void lnSystemTimerInit()
{
    // SystemCoreClock is in Hz; we want ticks per us
    // STK is configured with STCLK=1, counting at HCLK directly.
    tickPerUs = SystemCoreClock / (1000 * 1000); // ticks per us
}

/**
 * @brief Read the lower 32 bits of the STK cycle counter.
 * @return Current 32-bit counter value (wraps every ~30 s at 144 MHz).
 */
uint32_t lnGetCycle32()
{ return stk->CTN_LO; }

/**
 * @brief Atomically read the full 64-bit STK cycle counter.
 *        Uses the standard HI→LO→HI consistency check to avoid rollover
 *        corruption between the two 32-bit reads.
 * @return 64-bit counter value (wraps after many millennia).
 */
uint64_t lnGetCycle64()
{
    volatile uint32_t high, low;
    while (1)
    {
        high = stk->CTN_HI;
        low = stk->CTN_LO;
        volatile uint32_t high2 = stk->CTN_HI;
        if (high == high2)
            break;
    }
    uint64_t r = high;
    r <<= 32;
    r += low;
    return r;
}

/**
 * @brief Return the approximate number of microseconds since boot.
 *        Uses a division based on tickPerUs (HCLK / 1 MHz).
 * @return 32-bit µs counter (wraps after ~71 min at 144 MHz).
 */
uint32_t lnGetUs()
{
    // Simple approximation using the cycle counter
    uint64_t tick = lnGetCycle64();
    return (uint32_t)(tick / tickPerUs);
}

/**
 * @brief Return the approximate number of microseconds since boot (64-bit).
 *        Same as lnGetUs but without 32-bit wrap limit.
 * @return 64-bit µs counter.
 */
uint64_t lnGetUs64()
{
    uint64_t tick = lnGetCycle64();
    return tick / tickPerUs;
}

/**
 * @brief Busy-wait for the given number of microseconds.
 * @param wait  µs to wait (32-bit).
 */
void lnDelayUs(uint32_t wait)
{
    uint64_t target = lnGetUs64() + wait;
    while (lnGetUs64() < target)
        __asm__("nop");
}

/**
 * @brief Busy-wait for the given number of milliseconds.
 *        Uses the 1 ms SysTick interrupt counter.
 *        NOTE: hangs forever if the SysTick interrupt never fires.
 * @param wait  ms to wait (32-bit).
 */
void lnDelay(uint32_t wait)
{
    uint32_t next = sys_tick_count + wait;
    while (sys_tick_count < next)
    {
        asm("nop");
    }
}

/** Scratch register used for GDB / debugger access. */
volatile uint32_t lnScratchRegister = 0;

/**
 * @brief Initialise the STK timer and set a 1 ms compare.
 *        Must be called once before interrupts are enabled globally.
 *        The timer is configured in MODE_UP with STRE autoreload so that
 *        CNT resets to 0 on each CMP match, producing periodic 1 ms ticks
 *        without needing to re-arm CMP in the ISR.
 *
 *        Order: CMP_HI first, CMP_LO last (64-bit latch semantics).
 */
void init_timer_irq()
{
    // 8. Initialise timing constants (cycles-per-us, etc.)
    lnSystemTimerInit();

    // 9. Program the timer compare for 1 ms from now.
    //    WARNING: CMP_HI must be written FIRST, CMP_LO LAST (latches the
    //    64-bit value on many WCH timer peripherals).
    uint64_t now = stk->CTN_LO | ((uint64_t)stk->CTN_HI << 32);
    uint64_t cmp = now + (SystemCoreClock / 1000);
    stk->CMP_HI = (uint32_t)(cmp >> 32);
    stk->CMP_LO = (uint32_t)(cmp & 0xFFFFFFFF);

    // 11. Start the timer (both counter and interrupt enabled)
    stk->CTLR = SYSTICK_STEN | SYSTICK_STIE | SYSTICK_STCLK | SYSTICK_MODE_UP | SYSTICK_INIT | SYSTICK_STRE;
}
