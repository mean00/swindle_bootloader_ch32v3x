

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

/*
 * Points to the bottom of the stack, we should have 8 bytes free there
 */
extern uint32_t __msp_init;
uint64_t *marker = (uint64_t *)0x0000000020000000; // marker is at the beginning
extern bool check_fw();

/*
 * Clears reboot information so we reboot in "normal" mode
 */
static void clear_reboot_flags()
{
    *marker = 0;
}

/*
 * Returns whether we were rebooted into DFU mode
 */
static bool rebooted_into_dfu()
{
    return (*marker == 0xDEADBEEFCC00FFEEULL);
}

/**
    \fn check if the DFU pin(s) is grounded
 */
static bool check_forced_dfu()
{
    for (int i = 0; i < NB_DFUS; i++)
    {
        const lnPin pin = dfuPins[i];
        lnPinMode(pin, lnINPUT_PULLUP);
        for (int i = 0; i < 10; i++) // wait  a bit
            __asm__("nop");
        if (!lnDigitalRead(pin)) // "OK" Key pressed
            return true;
    }
    return false;
}
/**

*/
bool check_status()
{
    int go_dfu = false;
#define NEXT_STEP(x)                                                                                                   \
    {                                                                                                                  \
        if (!go_dfu)                                                                                                   \
            go_dfu |= (int)x;                                                                                          \
    }
    NEXT_STEP(rebooted_into_dfu());
    NEXT_STEP(check_forced_dfu());
    if (!go_dfu)
    {
        int fw_ko = 0;
        if (!check_fw())
        {
            fw_ko = 1;
        }
        go_dfu |= fw_ko;
    }
    clear_reboot_flags();
    //-- Force status
    go_dfu = true;
    //--
    return go_dfu;
}

//--
