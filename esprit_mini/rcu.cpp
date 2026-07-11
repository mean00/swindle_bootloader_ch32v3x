#include "esprit.h"

extern LN_RCU *arcu;

static void waitControlBit(int mask)
{
    while (!(arcu->CTL & mask))
        __asm__("nop");
}

static const uint8_t Multipliers[] = {
    0,  0,  0,  1,  // 0 1 2 3
    2,  3,  4,  5,  // 4 5 6 7
    6,  7,  8,  9,  // 8 9 10 11
    10, 11, 12, 13, // 12 13 14 15
    14, 14, 15      // 16 17 18 19
};

static void setPll(int multiplier, bool external)
{
    arcu->CFG1 = 0;
    int pllMultiplier = Multipliers[multiplier];

    uint32_t c0 = arcu->CFG0;
    // Clear PLLMUL [21:18], PLLSRC [16], PLLXTPRE [17] before setting them
    // (was: c0 &= (0xf << 18) which inadvertently cleared bits 0-17 and 22-31)
    c0 &= ~(LN_RCU_CFG0_PLLMUL_MASK | LN_RCU_CFG0_PLLSEL | LN_RCU_CFG0_PREDIV);
    c0 |= LN_RCU_CFG0_PLLMUL(pllMultiplier);

    if (external)
    {
        c0 |= LN_RCU_CFG0_PLLSEL; // HSE as PLL source
        // PLLXTPRE = 0: HSE not divided (8 MHz direct, target 144 MHz = 8×18)
    }
    else
    {
        c0 &= ~LN_RCU_CFG0_PLLSEL; // HSI/2 as PLL source
    }
    arcu->CFG0 = c0;

    arcu->CTL |= LN_RCU_CTL_PLLEN;
    waitControlBit(LN_RCU_CTL_PLLSTB);
}

uint32_t SystemCoreClock = 0;
static uint32_t _rcuClockApb1 = 0;
static uint32_t _rcuClockApb2 = 0;

// stage 1 already did most of the job
void lnInitSystemClock_limited()
{
    int inputClock = 8;
    int multiplier = 144 / inputClock; // 18

    SystemCoreClock = inputClock * multiplier * 1000000;
    _rcuClockApb1 = SystemCoreClock; // APB1 = sysclk/1
    _rcuClockApb2 = SystemCoreClock; // APB2 = sysclk/1
}
void lnInitSystemClock()
{
    // Switch to IRC8 first
    arcu->CTL |= LN_RCU_CTL_IRC8MEN;
    waitControlBit(LN_RCU_CTL_IRC8MSTB);

    uint32_t sysClock = arcu->CFG0;
    sysClock &= ~LN_RCU_CFG0_SYSCLOCK_MASK;
    arcu->CFG0 = sysClock;
    while ((arcu->CFG0 & LN_RCU_CFG0_SYSCLOCK_MASK) != LN_RCU_CFG0_SYSCLOCK_IRC8)
        ;

    // Disable PLL and XTAL
    arcu->CTL &= ~LN_RCU_CTL_HXTALEN;
    arcu->CTL &= ~LN_RCU_CTL_PLLEN;

    // Enable external crystal (8 MHz)
    arcu->CTL |= LN_RCU_CTL_HXTALEN;
    waitControlBit(LN_RCU_CTL_HXTASTB);

    // For 144 MHz: 8 MHz * 18 = 144 MHz
    int inputClock = 8;
    int multiplier = 144 / inputClock; // 18
    setPll(multiplier, true);

    SystemCoreClock = inputClock * multiplier * 1000000;
    _rcuClockApb1 = SystemCoreClock; // APB1 = sysclk/1
    _rcuClockApb2 = SystemCoreClock; // APB2 = sysclk/1

    // Setup AHB = sysclk:1, APB1 = AHB/1, APB2 = AHB/1, USB = PLL/3 (144MHz/3 = 48MHz)
    uint32_t clks = arcu->CFG0;
    clks &= ~LN_RCU_CFG0_AHB_MASK;   // clear AHB prescaler
    clks |= LN_RCU_CFG0_AHB_DIV(0);  // AHB = 1:1
    clks &= ~LN_RCU_CFG0_APB1_MASK;  // clear APB1 prescaler
    clks |= LN_RCU_CFG0_APB1_DIV(0); // APB1 = 1:1
    clks &= ~LN_RCU_CFG0_APB2_MASK;  // clear APB2 prescaler
    clks |= LN_RCU_CFG0_APB2_DIV(0); // APB2 = 1:1
    clks &= LN_RCU_CFG0_USBPSC_MASK; // clear USB prescaler
    clks |= LN_RCU_CFG0_USBPSC(2);   // USB = PLL/3 (48MHz)
    arcu->CFG0 = clks;

    // Configure flash wait states for 144 MHz (2 WS required for 72-144 MHz)
    // Must be set before increasing system clock frequency
    ((LN_FMC *)LN_FMC_ADR)->WS = LN_FMC_WS_LATENCY_2;

    // Switch to PLL
    clks &= ~LN_RCU_CFG0_SYSCLOCK_MASK;
    clks |= LN_RCU_CFG0_SYSCLOCK_PLL;
    arcu->CFG0 = clks;
    while ((arcu->CFG0 & LN_RCU_CFG0_SYSCLOCK_MASK) != LN_RCU_CFG0_SYSCLOCK_PLL)
        ;
}
