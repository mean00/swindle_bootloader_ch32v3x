

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

extern bool check_fw();
extern bool rebooted_into_dfu();
extern void jumpIntoApp();
extern void dfu();
extern void clearRebootedIntoDfu();

void lnRunTimeInit()
{
}

extern "C" void __attribute__((noinline)) xdeadEnd(int code)
{
    while (1)
    {
        __asm__("nop");
    }
}

extern "C" void vPortEnterCritical()
{
    deadEnd(0);
}

extern "C" void vPortExitCritical()
{
    deadEnd(0);
}
/**
 */
bool check_forced_dfu()
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
 *
 * @return
 */

void resetMe(const Peripherals periph)
{
    lnPeripherals::reset(periph);
    lnPeripherals::enable(periph);
}
void disabled(const Peripherals periph)
{
    lnPeripherals::disable(periph);
}
/**

*/
bool bootloader()
{

    // Activate GPIO B for now
    lnPeripherals::enable(pGPIOB);
    lnPeripherals::enable(pGPIOC);
    lnPeripherals::enable(pAF);

    // The LEDs are all on GPIO A
    resetMe(pGPIOA);
    resetMe(pGPIOB);
    resetMe(pGPIOC);

    // We need alternate functions too
    resetMe(pAF);

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
    clearRebootedIntoDfu();

    go_dfu = true;

    if (go_dfu == false)
    {
        jumpIntoApp();
    }
    lnCpuID::identify();
    dfu();
    return false;
}
/**
 * @brief
 *
 * @return int
 */
int main()
{
    bootloader();
}
//--
