
/**
 * @file dfu.cpp
 * @author mean00
 * @brief
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "class/dfu/dfu_device.h"
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnFMC.h"
#include "lnGPIO.h"
#include "lnPeripherals.h"
#include "swindle_bl.h"
#include "memory_config.h"
#include "pinout.h"
#include "usbd.h"
#include "ch32v30x.h"

//
#if 1
#define LNFMC_ERASE lnFMC::erase
#define LNFMC_WRITE lnFMC::write
#else
#define LNFMC_ERASE(...) true
#define LNFMC_WRITE(...) true
#endif


/**
 */


static bool led = false;
static uint32_t rendezvous;


uint32_t target_address;


/**
 * @brief
 *
 */
void dfu()
{       
    // sys tick
    setupSysTick();

    // enable 48 Mhz clock for usb FS
    lnPeripherals::enableUsb48Mhz();

    // enable USB
    lnPeripherals::enable(Peripherals::pUSBFS_OTG_CH32v3x);    

    // enable sysTick
    lnEnableInterrupt(LN_IRQ_SYSTICK);    
    printC("Entering DFU\n");
    // enable interrupt globally
    EnableIrqs();

    // board_init();
    tud_init(0);
    rendezvous = lnGetMs() + 200;
    while (1)
    {
        tud_task(); // tinyusb device task
        if (lnGetMs() > rendezvous)
        {
            rendezvous += 200;
            lnDigitalWrite(LED, led);
            lnDigitalWrite(LED2, led);
            led = !led;
        }
        // led_blinking_task();
    }
    deadEnd(0);
}
/**
 * @brief
 *
 * @param adr
 * @return true
 * @return false
 */
bool flashErase(uint32_t adr)
{
    if (adr < 8 * 1024) // dont write the bootloader
    {
        printC("Dont erase the BL!\n");
        return false;
    }
   // printCHex("erase", adr);
    return LNFMC_ERASE(adr, 1);
}
/**
 * @brief
 *
 * @param adr
 * @param data
 * @param size
 * @return true
 * @return false
 */
bool flashWrite(uint32_t adr, const uint8_t *data, int size)
{
    if (adr < (FLASH_BOOTLDR_SIZE_KB * 1024)) // dont write the bootloader
    {
      //  printC("Dont write the BL!\n");
        return false;
    }
    //printCHex("wr", adr);
    if (!LNFMC_WRITE(adr, data, size))
    {
     //   printCHex("write failed\n", adr);
        return false;
    }
    uint8_t *p = (uint8_t *)adr;
    bool correct = true;
    for (int i = 0; i < size; i++)
    {
        if (data[i] != p[i])
        {
            correct = false;
            printCHex("write verify failed at adr\n", adr + i);
            printCHex(" expected", p[i]);
            printCHex(" got ", data[i]);
            printC("\n");
        }
    }
    return correct;
}

/**
 * @brief flash write
 *
 */
extern "C" void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const *data, uint16_t length)
{
    int er = DFU_STATUS_OK;
    switch (block_num)
    {
    case 0: {
        if (length < 5)
        {
            printC("Incorrect len\n");
            er = DFU_STATUS_ERR_UNKNOWN;
            break;
        }
        uint32_t address = data[1] + (data[2] << 8) + (data[3] << 16) + (data[4] << 24);
        switch (data[0])
        {
        case 0x41: // erase
        {
            if (!flashErase(address))
                er = DFU_STATUS_ERR_ERASE;
            break;
        }
        case 0x21: // set address
        {
            //printC("setAdr\n");
            target_address = address;
            // return;
            break;
        }
        break;
        default:
            printC("CMD Err\n");
            er = DFU_STATUS_ERR_UNKNOWN;
            break;
        }
    }
    break;
    case 1:
        //printC("Block1 CB\n");
        break;
    default:
        //printC("other CB\n");
        uint32_t adr = target_address + (block_num - 2) * CFG_TUD_DFU_XFER_BUFSIZE;
        if (!flashWrite(adr, data, length))
        {
            printC("Flash Err\n");
            er = DFU_STATUS_ERR_WRITE;
        }
        break;
    }
    tud_dfu_finish_flashing(er);
}

/**
 * @brief
 *
 */
extern "C" void tud_dfu_detach_cb(void)
{
    printC("Detach CB\n");
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
    printC("Manifest CB\n");
    // nothing to do..
    tud_dfu_detach_cb();
}
/**
 * @brief
 *
 */
extern "C" uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
    return 10; // ??
}
// EOF
