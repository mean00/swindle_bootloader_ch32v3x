
#include "lnArduino.h"
#include "lnPeripheral_priv.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE];

extern void setup();
extern void loop();
extern "C" void _init();
void lnIrqSysInit();
void lnExtiSysInit();
void lnDmaSysInit();
void lnRunTimeInit();
void lnRunTimeInitPostPeripherals();
/**
 *
 * @param
 */
void initTask(void *)
{
    LoggerInit();
    setup();
    while(1)
    {
        loop();
    }
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
// dummy call to prevent the linker from removing it...
extern const uint32_t *lnGetFreeRTOSDebug();

extern void bootloader(void);

int main()
{
    //
    lnRunTimeInit();

    // The LEDs are all on GPIO A
    resetMe(pGPIOA);
    resetMe(pGPIOB);
    resetMe(pGPIOC);

    // We need alternate functions too
    resetMe(pAF);
    bootloader();

}
// EOF
