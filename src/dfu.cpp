
#include "lnArduino.h"
#include "lnGPIO.h"
#include "lnCpuID.h"
#include "lnPeripherals.h"
#include "usbd.h"
/**
*/
void dfu()
{
    // switch to higher clock
    lnInitSystemClock();
    // enable USB
    lnPeripherals::enable( Peripherals::pUSB);

    //board_init();
    tud_init(0);
    while (1)
    {
        tud_task(); // tinyusb device task
       // led_blinking_task();
    }
    deadEnd(0);
}

extern "C" const uint8_t  *tud_descriptor_device_cb()
{
    deadEnd(0);       
    return NULL;
}

extern "C" const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    deadEnd(0);
    return NULL;
}

extern "C" const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t lan)
{
    deadEnd(0);
    return NULL;
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

void xDelay(int a)
{
    deadEnd(0);
}