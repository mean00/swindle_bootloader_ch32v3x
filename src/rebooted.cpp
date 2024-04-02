/**
 *
 */

#include "lnArduino.h"
#include "memory_config.h"
#include "pinout.h"
#include "swindle_bl.h"

#define MARKER_DFU 0xDEADBEEFCC00FFEEULL
// Points to the bottom of the stack, we should have 8 bytes free there
static uint64_t *marker = (uint64_t *)0x0000000020000000; // marker is at the beginning

/**
 * @brief put the marker to force reboot to dfu
 * 
 */
void reboot_into_bootloader()
{
    *marker = MARKER_DFU;
}

// 
/**
 * @brief Clears reboot information so we reboot in "normal" mode
 * 
 */
void clear_reboot_flags()
{
    *marker = 0;
}
/**
 * @brief Returns whether we were rebooted into DFU mode
 * 
 * @return true 
 * @return false 
 */
bool rebooted_into_dfu()
{
    return (*marker == MARKER_DFU);
}
/**
 * @brief clear the reboot-to-dfu marker
 * 
 */
void clearRebootedIntoDfu()
{
    *marker = 0;
}

/**
 * \brief jump into the application, reverto to ~ post reset state
 */
void jumpIntoApp()
{
    // Switch on the LEDS before jumping
    lnPinMode(LED, lnOUTPUT);
    lnPinMode(LED2, lnOUTPUT);
    lnDigitalWrite(LED, false);
    lnDigitalWrite(LED2, false);
    // there must be a simpler way...
    DisableIrqs();
    SysTick_Stop();
    uartDeinit();
    lnDisableInterrupt(LN_IRQ_SYSTICK);    
    lnDisableInterrupt(LN_IRQ_OTG_FS);
    // disable FPU etc..
    asm volatile("csrw 0x804, x0\n"); // INTSYSCR : hw stack etc...
    clockDefault();
    __asm__( 
        "lui t0, 0x4\t\n"
        "jalr x0,0(t0)\n" ::);
}
