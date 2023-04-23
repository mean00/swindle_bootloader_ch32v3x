
#include "lnArduino.h"
#include "lnGPIO.h"
#include "lnCpuID.h"
#include "lnPeripherals.h"
#include "usbd.h"
#include "class/dfu/dfu_device.h"
#include "lnFMC.h"

/**
*/
extern void lnIrqSysInit();
extern void _enableDisable_direct(bool enableDisable, const int &irq_num);
extern void setupSysTick();
extern void EnableIrqs();

#define SysTicK_IRQn 12

extern void uartInit();
extern void uartSend(const char *c);

bool flashErase(uint32_t adr);
bool flashWrite(uint32_t adr, const uint8_t *data, int size);

uint32_t target_address;

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
    lnPeripherals::enable( Peripherals::pUART0);
    

    // enable sysTick
    _enableDisable_direct(true, SysTicK_IRQn);  

    //
    uartInit();
    uartSend("Going DFU\n");
    // enable interrupt globally
    EnableIrqs();

    uint32_t adr=220*1024+0x4000;
    flashErase(adr);
    uint8_t data[256];
    for(int i=0;i<256;i++) data [i] = i;
    flashWrite(adr,data,256);
    while(1)
    {
        __asm__("nop");
    }


    //board_init();
    tud_init(0);
    while (1)
    {
        tud_task(); // tinyusb device task
       // led_blinking_task();
    }
    deadEnd(0);
}
/**
*/
bool flashErase(uint32_t adr)
{
    if(adr<8*1024) // dont write the bootloader
    {
        uartSend("Dont erase the BL!\n");
        return false;
    }
    uartSend("page erase\n");
    return lnFMC::eraseCh32v3(adr,1);
    
}
/**
*/
bool flashWrite(uint32_t adr, const uint8_t *data, int size)
{
    if(adr<8*1024) // dont write the bootloader
    {
        uartSend("Dont write the BL!\n");
        return false;
    }
    uartSend("page write\n");
    return lnFMC::writeCH32V3(adr, data, size);    
}

/*
    Perform flashing...
*/
extern "C" void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const *data, uint16_t length)
{
    int er=DFU_STATUS_OK;
    switch(block_num)
    {
        case 0 : 
            {
                if(length<5)
                {
                    uartSend("Incorrect len\n");
                    er=DFU_STATUS_ERR_UNKNOWN;
                    break;
                }
                uint32_t address = data[1]+(data[2]<<8)+(data[3]<<16) + (data[4]<<24);
                switch(data[0])
                {
                    case 0x41: // erase
                    {
                        if( !flashErase(address))  er= DFU_STATUS_ERR_ERASE;
                        break;                     
                    }
                    case 0x21: // set address
                    {
                        uartSend("setAdr\n");
                        target_address=address;
                        break;
                    }
                    break;
                    default:
                        uartSend("CMD Err\n");
                        break;
                }
            }
            break;
        case 1 : uartSend("Block1 CB\n");break;
        default: uartSend("other block CB\n");break;
    }
    tud_dfu_finish_flashing(er);
}
/*
    Called AFTER the flashing has been done
*/
extern "C" void tud_dfu_manifest_cb(uint8_t alt)
{
    uartSend("Manifest CB\n");
    // nothing to do..
}

extern "C" void tud_dfu_detach_cb(void)
{
    uartSend("Detach CB\n");
    // do reset 
}


extern "C" uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
   return 10; // ??
}

