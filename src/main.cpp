

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

extern bool check_fw();
extern bool rebooted_into_dfu();
extern void jumpIntoApp();
extern void dfu();
extern void clearRebootedIntoDfu();
extern void lnIrqSysInit();
extern void uartInit();
extern void DisableIrqs();

// This move the stack further
uint8_t ucHeap[8*1024];

//
#if 0
#define printC(...)                                                                                                    \
    {                                                                                                                  \
    }
#define printCHex(...)                                                                                                 \
    {                                                                                                                  \
    }
#else
extern void printC(const char *c);
extern void printCHex(const char *c, uint32_t val_in_hex);
#endif


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
    lnPinMode(FORCED_DFU_PIN, lnINPUT_PULLUP);
    for (int i = 0; i < 10; i++) // wait  a bit
        __asm__("nop");
    if (!lnDigitalRead(FORCED_DFU_PIN)) // "OK" Key pressed
        return true;
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
    DisableIrqs();
    // switch to higher clock
    lnInitSystemClock();
    // setup interrupts
    lnIrqSysInit();
    // Activate GPIO B for now
    lnPeripherals::enable(pGPIOA);
    lnPeripherals::enable(pGPIOB);
    lnPeripherals::enable(pGPIOC);
    lnPeripherals::enable(pAF);
    lnPeripherals::enable(Peripherals::pUART0);

    // The LEDs are all on GPIO A
    resetMe(pGPIOA);
    resetMe(pGPIOB);
    resetMe(pGPIOC);    
    resetMe(pAF); // We need alternate functions too
//
    uartInit();
    printC("Checking FW\n");
    lnPinMode(LED, lnOUTPUT);
    lnPinMode(LED2, lnOUTPUT);

    //
    


    int go_dfu = false;
#define NEXT_STEP(x)                                                                                                   \
    {                                                                                                                  \
        if (!go_dfu)  {                                                                                                \
            go_dfu |= (int)x;                                                                                          \
            printC(#x); printC("\n");                                                                                   \
        } \
    }
    NEXT_STEP(rebooted_into_dfu());
    NEXT_STEP(check_forced_dfu());    
    if (!go_dfu)
    {
        int fw_ko = 0;
        if (!check_fw())
        {
            printC("Hash Ko!\n");
            fw_ko = 1;
        }
        go_dfu |= fw_ko;
    }
    clearRebootedIntoDfu();

    //go_dfu = true;

    if (go_dfu == false)
    {
        printC("Jumping into app\n");
        jumpIntoApp();
    }
    lnCpuID::identify();
    printC("Going DFU\n");
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
