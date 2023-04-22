
#include "lnArduino.h"
#include "lnGPIO.h"
#include "lnCpuID.h"
#include "lnPeripherals.h"
#include "usbd.h"

/**
*/
extern void lnIrqSysInit();
extern void _enableDisable_direct(bool enableDisable, const int &irq_num);
extern void setupSysTick();
extern void EnableIrqs();

#define SysTicK_IRQn 12


/**
*/
void dfu()
{
    // switch to higher clock
    lnInitSystemClock();

    // sys tick
    setupSysTick();
    
    // setup interrupts
    lnIrqSysInit();
    
    // enable 48 Mhz
    lnPeripherals::enableUsb48Mhz();

    // enable USB
    lnPeripherals::enable( Peripherals::pUSBFS_OTG_CH32v3x);

    // enable sysTick
    _enableDisable_direct(true, SysTicK_IRQn);  

    // enable interrupt globally
    EnableIrqs();

    //board_init();
    tud_init(0);
    while (1)
    {
        tud_task(); // tinyusb device task
       // led_blinking_task();
    }
    deadEnd(0);
}




extern "C" void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const *data, uint16_t length)
{
    deadEnd(0);
}

extern "C" void tud_dfu_manifest_cb(uint8_t alt)
{
    deadEnd(0);
}

extern "C" uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
    deadEnd(0);
    return 0;
}
