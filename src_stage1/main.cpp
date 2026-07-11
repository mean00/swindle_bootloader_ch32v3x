/**
 * @file    main.cpp
 * @brief   Stage 1 bootloader for CH32V3x (RISC-V)
 *
 * @details This is the very first code that runs after the CPU reset vector.
 *          Its responsibilities include:
 *            1. Clock initialisation (144 MHz) and GPIO setup.
 *            2. DFU decision logic — checks a dedicated DFU pin, a
 *               reboot-marker in RAM, and CRC32 validity of the
 *               application image in flash.
 *            3. If DFU mode is required: copies the stage-2 payload from
 *               flash to RAM and chains to it.
 *            4. If the application image is valid: chains directly to the
 *               application entry point.
 *
 * @note    The stage-2 payload is linked at 0x20004000 (starting at 16 KB of RAM)
 *          and is embedded as a binary blob via @c stage2_payload.h.
 *
 * @ingroup stage1
 */

#include "esprit.h"
#include "flash_config.h"
#include "pinout.h"
#include "registers.h"
#include "stage2_payload.h"

// Forward declarations
/** @brief CRC32 computation using the CH32 hardware CRC peripheral. */
extern "C" uint32_t ch32_crc(uint32_t addr, uint32_t len_in_u32);

/** @brief RAM marker pointer used to signal reboot-into-DFU. */
static uint64_t *marker = (uint64_t *)RAM_START_ADDR;

/**
 * @brief  Write the reboot-into-DFU magic value into the RAM marker.
 * @note   The next boot will see the marker and enter DFU mode.
 */
static void reboot_into_bootloader()
{ *marker = 0xDEADBEEFCC00FFEEULL; }

/**
 * @brief  Clear the reboot marker so a normal boot proceeds.
 */
static void clear_reboot_flags()
{ *marker = 0; }

/**
 * @brief  Check whether we are in DFU mode due to a prior reboot-request.
 * @return true if the magic marker is present, false otherwise.
 */
static bool rebooted_into_dfu()
{ return (*marker == 0xDEADBEEFCC00FFEEULL); }

/**
 * @brief  Chain (jump) execution to an arbitrary address in RISC-V.
 *
 * Sets the stack pointer to the top of RAM and performs a tail-call
 * via a register jump.  The target image is plain code without a
 * vector table — for stage-2 the entry function sets its own SP.
 *
 * @param addr  The absolute address to jump to (e.g. 0x20004000).
 */
static void chain_to(uint32_t addr)
{
    __asm volatile("li sp, 0x20010000\n"
                   "mv t0, %0\n"
                   "jr t0\n"
                   :
                   : "r"(addr)
                   :);
}
/**
 *
 */
static void toggle_led()
{
    static bool state = false;
    state = !state;
    lnDigitalWrite(LED_PIN, state);
}
#define RCC_CTLR (*(volatile uint32_t *)0x40021000UL)
#define RCC_CFGR0 (*(volatile uint32_t *)0x40021004UL)
#define RCC_INTR (*(volatile uint32_t *)0x40021008UL)

static void revert_clock_to_hsi(void)
{
    // 1. Enable HSI (Internal 8MHz oscillator)
    RCC_CTLR |= 0x00000001; // Set HSION bit

    // 2. Wait until HSI is ready
    while ((RCC_CTLR & 0x00000002) == 0)
        ; // Check HSIRDY bit

    // 3. Reset CFGR0 to default (select HSI as system clock, reset prescalers)
    RCC_CFGR0 = 0x00000000;

    // 4. Wait until HSI is used as system clock
    while ((RCC_CFGR0 & 0x0000000C) != 0x00000000)
        ; // Check SWS bits

    // 5. Disable HSE, CSS, PLL, PLL2, PLL3
    RCC_CTLR &= 0xFEF6FFFF; // Clear HSEON, CSSON, PLLON

    // 6. Reset HSEBYP
    RCC_CTLR &= 0xFFFBFFFF;

    // 7. Clear all clock interrupts
    RCC_INTR = 0x009F0000;
}

/**
 * @brief  Stage 1 main entry point.
 *
 * Initialises the system, decides whether to chain to the
 * application or to DFU mode, and acts accordingly.
 *
 * @return This function should never return — it either chains
 *         to the application, chains to stage-2 DFU, or hangs.
 */
static uint32_t go_dfu = 0;
int main(void)
{
    // Initialize system clock (144 MHz)
    lnInitSystemClock();

    // Enable GPIO clocks
    lnPeripherals::enable(pGPIOA);
    lnPeripherals::enable(pGPIOB);
    lnPeripherals::enable(pGPIOC);

    // Setup LED
    lnPinMode(LED_PIN, lnOUTPUT);
    lnDigitalWrite(LED_PIN, false);

    // Setup DFU pin
    lnPinMode(DFU_PIN, lnINPUT_PULLUP);

    // LED on briefly to indicate bootloader start
    toggle_led();

    // Small delay for pin states to settle
    for (int i = 0; i < 1000; i++)
        __asm__("nop");

    // Check 1: was a reboot-to-DFU requested?
    if (rebooted_into_dfu())
        go_dfu |= 1;

    // Check 2: is the DFU pin grounded?
    if (!lnDigitalRead(DFU_PIN))
        go_dfu |= 2;

    // Read app header at flash offset APP_ADDRESS
    const uint32_t app_start = APP_ADDRESS;
    const uint32_t *const app_base = (uint32_t *)app_start;
    uint32_t sig = app_base[0]; // stack pointer (should be in RAM range)
    uint32_t imageSize = app_base[1];
    uint32_t checksum = app_base[2];

    // Check 4: CRC32 validity
    if (!go_dfu && imageSize > 0 && imageSize < 256 * 1024)
    {
        if (imageSize != 0x1234 || checksum != 0x5678) // valid but no hash
        {
            uint32_t computed = ch32_crc((uint32_t)&(app_base[3]), imageSize >> 2);
            if (computed != checksum)
                go_dfu |= 8;
        }
    }
    else if (!go_dfu)
    {
        go_dfu |= 16; // absurd size
    }

    clear_reboot_flags();

    toggle_led();

    // go_dfu = 1;

    if (!go_dfu)
    {
        // Revert clock to default HSI before jumping
        revert_clock_to_hsi();

        // Jump to application
        chain_to(app_start);
    }

// Need DFU mode: copy stage2 to RAM and chain
// Stage2 payload is embedded as a binary blob (via stage2_payload.h)
// Stage2 is linked at 0x20004000 (starting at 16 KB offset in RAM)
#define STAGE2_RAM_ADDR 0x20004000
    memcpy((void *)STAGE2_RAM_ADDR, (const void *)stage2_payload, (size_t)stage2_payload_size);

    // LED on to indicate DFU mode
    toggle_led();

    chain_to(STAGE2_RAM_ADDR);

    // Should never reach here
    xAssert(0);
    return 0;
}
