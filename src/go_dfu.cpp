
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
#include "lnUSBD.h"
#include "lnUsbDFU.h"
#include "lnUsbStack.h"
#include "memory_config.h"

#include "pinout.h"

#include "usbd.h"
//
#if 1
#define LNFMC_ERASE lnFMC::erase
#define LNFMC_WRITE lnFMC::write
#else
#define LNFMC_ERASE(...) true
#define LNFMC_WRITE(...) true
#endif

#define printC(...)                                                                                                    \
    {                                                                                                                  \
    }
#define printCHex(...)                                                                                                 \
    {                                                                                                                  \
    }

static void dfu_download_cb(/*uint8_t alt,*/ uint32_t block_num, uint8_t const *data, uint32_t length);
uint32_t target_address;
bool led = false;

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
#define ALT_COUNT 1
enum
{
    ITF_NUM_DFU_MODE,
    ITF_NUM_TOTAL
};
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_DFU_DESC_LEN(ALT_COUNT))

#define FUNC_ATTRS (DFU_ATTR_CAN_DOWNLOAD | DFU_ATTR_MANIFESTATION_TOLERANT)

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, Alternate count, starting string index, attributes, detach timeout, transfer size
    TUD_DFU_DESCRIPTOR(ITF_NUM_DFU_MODE, ALT_COUNT, 4, FUNC_ATTRS, 1000, CFG_TUD_DFU_XFER_BUFSIZE),
};
//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "lnBMP",                    // 1: Manufacturer
    "lnBMP CH32-DFUv2.0",       // 2: Product
    "123456",                   // 3: Serials, should use chip ID
    FLASH_DFU_STRING,
    //"lnBMP_FW",                       // 4: DFU Partition 1
};
//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {.bLength = sizeof(tusb_desc_device_t),
                                        .bDescriptorType = TUSB_DESC_DEVICE,
                                        .bcdUSB = 0x0200,
                                        .bDeviceClass = 0x00,
                                        .bDeviceSubClass = 0x00,
                                        .bDeviceProtocol = 0x00,
                                        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                        .idVendor = 0x1d50,
                                        .idProduct = LN_ID_PRODUCT,
                                        .bcdDevice = 0x0100,
                                        .iManufacturer = 0x01,
                                        .iProduct = 0x02,
                                        .iSerialNumber = 0x03,
                                        .bNumConfigurations = 0x01};

/**
 */
static void helloUsbEvent(void *cookie, lnUsbStack::lnUsbStackEvents event)
{
}

/**
 * @brief flash write
 *
 */
void dfu_download_cb(/*uint8_t alt,*/ uint32_t block_num, uint8_t const *data, uint32_t length)
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
            if (address < (FLASH_BOOTLDR_SIZE_KB * 1024))
            {
                printC("Incorrect address \n");
                er = DFU_STATUS_ERR_WRITE;
            }
            else if (!LNFMC_ERASE(address, 1)) // erase 1 KB
                er = DFU_STATUS_ERR_ERASE;
            break;
        }
        case 0x21: // set address
        {
            printC("setAdr\n");
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
        printC("Block1 CB\n");
        break;
    default:
        printC("other CB\n");
        uint32_t adr = target_address + (block_num - 2) * CFG_TUD_DFU_XFER_BUFSIZE;
        if (adr < (FLASH_BOOTLDR_SIZE_KB * 1024))
        {
            printC("Incorrect address \n");
            er = DFU_STATUS_ERR_WRITE;
        }
        else if (!LNFMC_WRITE(adr, data, length))
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

extern "C" void tusb_init();
extern "C" void tud_task();

bool go_dfu()
{
    for (int i = 0; i < NB_LEDS; i++)
    {
        lnPinMode(ledPins[i], lnOUTPUT);
    }

    Logger("Going DFU 1\n");
    //
    lnUsbStack *usb = new lnUsbStack;
    usb->init(5, string_desc_arr);
    usb->setConfiguration(desc_configuration, desc_configuration, &desc_device, NULL);
    usb->setEventHandler(NULL, helloUsbEvent);
    lnUsbDFU::addDFURTCb(&dfu_download_cb);
#define LED_BLINK_PERIOD 100
    // bypass usb task, we run tinyusb in non freertos mode to gain some flash space
    tusb_init();
    uint32_t rendez_vous = lnGetMs() + LED_BLINK_PERIOD;
    while (1)
    {
        uint32_t newClock = lnGetMs();
        if (newClock > rendez_vous)
        {
            rendez_vous = newClock + LED_BLINK_PERIOD;
            for (int i = 0; i < NB_LEDS; i++)
            {
                lnDigitalToggle(ledPins[i]);
            }
        }
        tud_task();
    }
}
// EOF
