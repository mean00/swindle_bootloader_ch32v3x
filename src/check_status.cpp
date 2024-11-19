

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

/*
 * Points to the bottom of the stack, we should have 8 bytes free there
 */
extern uint32_t __msp_init;
uint64_t *marker = (uint64_t *)0x0000000020000000ULL; // marker is at the beginning
extern bool check_fw();

/*
 * Returns whether we were rebooted into DFU mode
 */
static bool rebooted_into_dfu()
{
    bool rebooted = (*marker == 0xDEADBEEFCC00FFEEULL);
    *marker = 0;
    return rebooted;
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

#define NEXT_STEP(x)                                                                                                   \
    {                                                                                                                  \
        if (!go_dfu)                                                                                                   \
            go_dfu |= (int)x;                                                                                          \
    }

/**

*/
bool check_status()
{
    int go_dfu = 0;
    NEXT_STEP(rebooted_into_dfu());
    NEXT_STEP(check_forced_dfu());
    if (!go_dfu)
    {
        if (!check_fw())
        {
            go_dfu |= 4;
        }
    }

    //-- Force status
    // go_dfu = 1;
    //--
    return go_dfu != 0;
}

//--
