/**
 * @file main.cpp
 * @author mean00
 * @brief  Main function for CH32 bootloader
 * @version 0.1
 * @date 2024-04-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"
#include "swindle_bl.h"


// This move the stack further
uint8_t ucHeap[8*1024];

/**
 * @brief Check if the "forced DFU" pin is grounded
 * 
 * @return true 
 * @return false 
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

#define NEXT_STEP(x)                                                                                                   \
    {                                                                                                                  \
        if (!go_dfu)  {                                                                                                \
            go_dfu |= (int)x;                                                                                          \
            printC(#x); printC("\n");                                                                                   \
        } \
    }


/** 
    \brief core bootloader check, if checks fail, jump to DFU
*/
int main()
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
    
    resetMe(pGPIOA);
    resetMe(pGPIOB);
    resetMe(pGPIOC);    
    resetMe(pAF); // We need alternate functions too

    lnCpuID::identify();
    uartInit();
    printC("Checking FW\n");
    lnPinMode(LED, lnOUTPUT);
    lnPinMode(LED2, lnOUTPUT);

    int go_dfu = false;
    NEXT_STEP(rebooted_into_dfu()); // is the magic marker in ram ?
    NEXT_STEP(check_forced_dfu());  // is the DFU pin grounded ?
    if (!go_dfu)
    {        
        if (!check_fw())
        {
            printC("Hash Ko!\n");
            go_dfu = true;
        }
        
    }
    clearRebootedIntoDfu(); // remove magic marker
    if (go_dfu == false)
    {
        printC("Jumping into app\n");
        jumpIntoApp();
    }
    printC("Going DFU\n");
    dfu();
    return 0;
}
//--
