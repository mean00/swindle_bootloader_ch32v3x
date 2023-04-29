
#include "lnArduino.h"
#include "lnGPIO.h"
#include "lnCpuID.h"
#include "lnPeripherals.h"
#include "usbd.h"
#include "class/dfu/dfu_device.h"
#include "lnFMC.h"
/**

*/
#define LED  PC13
#define LED2 PA8

//#define NO_FLASH

/**
*/
extern void lnIrqSysInit();
extern void _enableDisable_direct(bool enableDisable, const int &irq_num);
extern void setupSysTick();
extern void EnableIrqs();
extern void systemReset();

#define SysTicK_IRQn 12

extern void uartInit();
extern void uartSend(const char *c);

bool flashErase(uint32_t adr);
bool flashWrite(uint32_t adr, const uint8_t *data, int size);
uint32_t lnGetMs();

uint32_t target_address;

bool led=false;
uint32_t rendezvous;

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
    lnPinMode(LED, lnOUTPUT);
    lnPinMode(LED2, lnOUTPUT);

    //
    uartInit();
    uartSend("Going DFU\n");
    // enable interrupt globally
    EnableIrqs();
    
    //board_init();
    tud_init(0);
    rendezvous=lnGetMs()+200;
    while (1)
    {
        tud_task(); // tinyusb device task
        if(lnGetMs()>rendezvous)
        {
            rendezvous+=200;
            lnDigitalWrite(LED,led);
            lnDigitalWrite(LED2,led);
            led=!led;
        }
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
#ifdef NO_FLASH    
    return true;
#else    
    return lnFMC::eraseCh32v3(adr,1);
#endif    
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
#ifdef NO_FLASH    
    return true;
#else
    return lnFMC::writeCH32V3(adr, data, size);
#endif    
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
                        //return;
                        break;
                    }
                    break;
                    default:
                        uartSend("CMD Err\n");
                        er=DFU_STATUS_ERR_UNKNOWN;
                        break;
                }
            }
            break;
        case 1 : uartSend("Block1 CB\n");break;
        default: 
                uartSend("other block CB\n");
                uint32_t adr=target_address+(block_num-2)*CFG_TUD_DFU_XFER_BUFSIZE;
                if(!flashWrite(adr,data,length))
                {
                    uartSend("Flash Err\n");
                    er=DFU_STATUS_ERR_WRITE;
                }        
                break;
    }
    tud_dfu_finish_flashing(er);
}
/**
*/

extern "C" void tud_dfu_detach_cb(void)
{
    uartSend("Detach CB\n");
    // do reset 
    // CH32V2x and CH32V3x reset the system by
    // setting the SYSRESET bit in the interrupt configuration register (PFIC_CFGR) to 1, or by setting the
    // SYSRESET bit in the PFIC_SCTLR
    systemReset();
    
}
/*
    Called AFTER the flashing has been done
*/
extern "C" void tud_dfu_manifest_cb(uint8_t alt)
{
    uartSend("Manifest CB\n");
    // nothing to do..
    tud_dfu_detach_cb();
}

extern "C" uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
   return 10; // ??
}

