/**
 * @file    dfu.cpp
 * @brief   DFU state machine for the CH32V3x stage-2 bootloader (DfuSe).
 *
 * @details Implements the USB DFU class request handler and flash
 *          programming logic using the ST DfuSe protocol (string-descriptor
 *          memory layout, no binary queries).  The handler supports:
 *            - DNLOAD block N: firmware data written at offset N×4096
 *            - Zero-length DNLOAD: manifest (end of download)
 *            - DFU_DETACH with wTimeout=0: jump to application
 *
 *          Upload (read-back) is intentionally NOT supported — returns
 *          empty for all blocks.
 *
 * @ingroup stage2
 */

#include "esprit.h"
#include "flash.h"
#include "flash_config.h"
#include "pinout.h"
#include "registers.h"
#include "usb.h"

extern void init_timer_irq();
extern uint32_t getTickCount();

static enum dfu_state state = STATE_DFU_IDLE;
static enum dfu_status status = DFU_STATUS_OK;
static uint32_t dfu_address = 0;
static uint32_t dfu_block_num = 0;
static uint32_t flash_end = 0;

/** @brief ST DfuSe special command: set target address for subsequent operations. */
#define DFUSE_CMD_SET_ADDRESS 0x21
/** @brief ST DfuSe special command: erase 256-byte page at given address. */
#define DFUSE_CMD_ERASE_PAGE 0x41

// ========== DFU request handler ==========

/**
 * @brief  Handle a USB DFU class-specific request (dFuse protocol).
 *
 * Dispatches DFU requests (DETACH, DNLOAD, UPLOAD, GETSTATUS, etc.)
 * and updates the DFU state machine accordingly.
 *
 * @param req   Pointer to the USB setup packet.
 * @param data  Output: pointer to response data (may be NULL).
 * @param len   Output: length of response data.
 * @return int  1 if the request was handled, 0 if not recognised.
 */
int dfu_handle_request(const struct usb_setup_packet *req, const uint8_t **data, uint16_t *len)
{
    *data = NULL;
    *len = 0;
    int ret = 0;

    switch (req->bRequest)
    {
        case DFU_DETACH:
        {
            uint16_t timeout = req->wValue;
            if (timeout == 0)
            {
                // Jump to application immediately
                status = DFU_STATUS_OK;
                state = STATE_APP_DETACH;
            }
            else
            {
                // Non-zero timeout: signal detach, the main loop will
                // jump to the application after the timeout expires
                state = STATE_APP_DETACH;
                status = DFU_STATUS_OK;
            }
            ret = 1;
            break;
        }

        case DFU_DNLOAD:
        {
            uint16_t block = req->wValue;
            uint16_t length = req->wLength;

            if (length == 0)
            {
                // Manifest: end of download
                state = STATE_DFU_MANIFEST_SYNC;
                status = DFU_STATUS_OK;
            }
            else
            {
                // Data will follow via OUT stage
                dfu_block_num = block;
                state = STATE_DFU_DNLOAD_SYNC;
                status = DFU_STATUS_OK;
            }
            ret = 1;
            break;
        }

        case DFU_UPLOAD:
        {
            // Read-back not supported — return empty for all blocks
            *data = NULL;
            *len = 0;
            state = STATE_DFU_UPLOAD_IDLE;
            status = DFU_STATUS_OK;
            ret = 1;
            break;
        }

        case DFU_GETSTATUS:
        {
            if (state == STATE_DFU_DNLOAD_SYNC)
            {
                state = STATE_DFU_DNBUSY;
            }
            else if (state == STATE_DFU_DNBUSY)
            {
                state = STATE_DFU_DNLOAD_IDLE;
            }
            else if (state == STATE_DFU_MANIFEST_SYNC)
            {
                state = STATE_DFU_MANIFEST;
            }

            static uint8_t resp[6];
            resp[0] = status;
            resp[1] = 10; // bwPollTimeout LSB (10ms)
            resp[2] = 0;
            resp[3] = 0;
            resp[4] = state;
            resp[5] = 0;
            *data = resp;
            *len = 6;
            ret = 1;
            break;
        }

        case DFU_CLRSTATUS:
            status = DFU_STATUS_OK;
            state = STATE_DFU_IDLE;
            ret = 1;
            break;

        case DFU_GETSTATE:
        {
            static uint8_t sr = STATE_DFU_IDLE;
            sr = state;
            *data = &sr;
            *len = 1;
            ret = 1;
            break;
        }

        case DFU_ABORT:
            state = STATE_DFU_IDLE;
            status = DFU_STATUS_OK;
            ret = 1;
            break;

        default: break;
    }
    return ret;
}

// ========== Data handler ==========

void dfu_handle_data(const uint8_t *data, uint16_t len)
{
    // Block 0 carries ST DfuSe special commands
    if (dfu_block_num == 0 && len >= 5)
    {
        uint8_t cmd = data[0];
        uint32_t addr =
            (uint32_t)data[1] | ((uint32_t)data[2] << 8) | ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 24);

        if (cmd == DFUSE_CMD_SET_ADDRESS)
        {
            dfu_address = addr;
            status = DFU_STATUS_OK;
        }
        else if (cmd == DFUSE_CMD_ERASE_PAGE)
        {
            lnDigitalWrite(LED_PIN, true);
            // Erase the 256-byte page containing this address
            if (!fmc_erase_page(addr & ~(FLASH_SECTOR_SIZE - 1)))
            {
                status = DFU_STATUS_ERR_ERASE;
                state = STATE_DFU_ERROR;
            }
            else
            {
                status = DFU_STATUS_OK;
            }
            lnDigitalWrite(LED_PIN, false);
        }
        else
        {
            status = DFU_STATUS_ERR_UNKNOWN;
            state = STATE_DFU_ERROR;
        }
        return;
    }

    // Block 2+ carries firmware data at dfu_address + (block-2) * TRANSFER_SIZE
    if (dfu_block_num >= 2 && len > 0)
    {
        uint32_t wa = dfu_address + (dfu_block_num - 2) * (uint32_t)DFU_TRANSFER_SIZE;

        if (wa + len > flash_end)
        {
            status = DFU_STATUS_ERR_ADDRESS;
            state = STATE_DFU_ERROR;
            return;
        }

        // Erase pages that aren't already erased
        for (uint32_t a = wa & ~(FLASH_SECTOR_SIZE - 1); a < wa + len; a += FLASH_SECTOR_SIZE)
        {
            if (!fmc_page_is_erased(a))
            {
                if (!fmc_erase_page(a))
                {
                    status = DFU_STATUS_ERR_ERASE;
                    state = STATE_DFU_ERROR;
                    return;
                }
            }
        }

        if (!fmc_write(wa, data, len))
        {
            status = DFU_STATUS_ERR_WRITE;
            state = STATE_DFU_ERROR;
            return;
        }

        if (!fmc_verify(wa, data, len))
        {
            status = DFU_STATUS_ERR_VERIFY;
            state = STATE_DFU_ERROR;
            return;
        }
        status = DFU_STATUS_OK;
        // state remains STATE_DFU_DNLOAD_SYNC
    }
}

// ========== Stage 2 entry ==========

extern void system_reset(void);
/*
 *
 */
static void toggle_led()
{
    static bool state = false;
    state = !state;
    lnDigitalWrite(LED_PIN, state);
}

/**
 * @brief  Stage-2 main entry — initialises hardware and runs the DFU poll loop.
 *
 * Sets up GPIOs, enables the USB peripheral, initialises the USB stack,
 * and enters an infinite loop polling for USB/DFU events.
 *
 * State transitions handled in the main loop:
 *   - MANIFEST_SYNC → MANIFEST_WAIT_RESET → system_reset()
 *   - APP_DETACH → jump to application
 */
void stage2_main(void)
{

    // Initialise SysTick (1ms interrupt) and enable global interrupts
    init_timer_irq();

    lnPeripherals::enable(pGPIOA);
    lnPeripherals::enable(pGPIOB);
    lnPeripherals::enable(pGPIOC);
    lnPeripherals::enable(pUSBFS_OTG_CH32v3x);

    lnPinMode(LED_PIN, lnOUTPUT);
    toggle_led();

    flash_end = FLASH_START_ADDR + (FLASH_BOOTLDR_SIZE_KB + FLASH_APP_SIZE_KB) * 1024;
    lnEnableInterrupt(LN_IRQ_SYSTICK);
    lnInterrupts();

    usb_init();
    uint32_t rdv = 0, now;
    while (1)
    {
        now = getTickCount();
        if (now > rdv)
        {
            rdv += 100;
            toggle_led();
        }
        do_usb_poll();

        if (state == STATE_DFU_MANIFEST_SYNC)
        {
            // All done — transition to wait-for-reset
            state = STATE_DFU_MANIFEST_WAIT_RESET;
        }

        if (state == STATE_DFU_MANIFEST_WAIT_RESET)
        {
            // Give the USB status phase a moment to complete, then reset
            system_reset();
        }

        if (state == STATE_APP_DETACH)
        {
            system_reset();
        }
    }
}
