/**
 * @file    usb.cpp
 * @brief   Register-level USB OTG FS driver for CH32V3x (WCH USB FS IP).
 *
 * @details Implements a polled USB device driver using the WCH USB FS
 *          peripheral at base address 0x50000000.  The driver:
 *            - Enumerates as a USB DFU device.
 *            - Handles standard requests (GET_DESCRIPTOR, SET_ADDRESS, etc.).
 *            - Routes DFU class requests to @c dfu_handle_request().
 *            - Manages EP0 control transfers (data IN/OUT, status).
 *            - Handles bus reset events and suspend.
 *
 * @note   All USB operations are polled — no USB interrupts are used.
 *         Only SysTick uses the interrupt system for 1ms timing.
 *
 * @ingroup stage2
 */

#include "usb.h"
#include "dfu_name.h"
#include "esprit.h"
#include "flash_config.h"
#include "registers.h"
#include "usb_vid_pid.h"
//
#include "printf.h"
// ========== USB descriptor data ==========

/** @brief String descriptor indices used in the USB descriptors. */
#define STRIDX_MFR 1     /**< Manufacturer string.       */
#define STRIDX_PRODUCT 2 /**< Product name string.       */
#define STRIDX_SERIAL 3  /**< Serial number string.      */
#define STRIDX_DFUSE 4   /**< DfuSe memory layout string. */

/** @brief EP0 maximum packet size. */
#define EP0_MAX_PACKET 64

static const struct usb_device_descriptor dev_desc = {.bLength = USB_DT_DEVICE_SIZE,
                                                      .bDescriptorType = USB_DT_DEVICE,
                                                      .bcdUSB = USB_VERSION_1_1,
                                                      .bDeviceClass = 0x00,
                                                      .bDeviceSubClass = 0x00,
                                                      .bDeviceProtocol = 0x00,
                                                      .bMaxPacketSize0 = EP0_MAX_PACKET,
                                                      .idVendor = USB_VID,
                                                      .idProduct = USB_PID,
                                                      .bcdDevice = 0x0100,
                                                      .iManufacturer = STRIDX_MFR,
                                                      .iProduct = STRIDX_PRODUCT,
                                                      .iSerialNumber = STRIDX_SERIAL,
                                                      .bNumConfigurations = 1};

static const struct usb_dfu_descriptor dfu_func_desc = {.bLength = sizeof(struct usb_dfu_descriptor),
                                                        .bDescriptorType = DFU_FUNCTIONAL,
                                                        .bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_CAN_UPLOAD |
                                                                        USB_DFU_MANIFEST_TOLERANT | USB_DFU_WILL_DETACH,
                                                        .wDetachTimeout = 1000,
                                                        .wTransferSize = DFU_TRANSFER_SIZE,
                                                        .bcdDFUVersion = 0x0110};

// ========== USB transfer state ==========

// EP0 DMA buffer area (must be 4-byte aligned)
static uint8_t ep0_buffer[EP0_MAX_PACKET] __attribute__((aligned(4)));
static uint8_t dfu_rx_buffer[DFU_TRANSFER_SIZE] __attribute__((aligned(4)));
static uint16_t dfu_rx_offset = 0;

// Control transfer state machine
enum ctrl_phase
{
    CTRL_IDLE,
    CTRL_DATA_IN,
    CTRL_DATA_OUT,
    CTRL_STATUS
};
static enum ctrl_phase ctrl_phase = CTRL_IDLE;
static const uint8_t *ctrl_data_ptr = NULL;
static uint16_t ctrl_data_len = 0;
static uint16_t ctrl_data_remaining = 0;
static uint8_t ctrl_req_type = 0;
static uint8_t ctrl_request = 0;
static uint8_t usb_dev_addr = 0;
static bool ep0_tog = false; // data toggle for EP0

// ========== Low-level helpers ==========

static void ep0_tx_nak(void)
{ EP_TX_CTRL(0) = USBFS_EP_RES_NAK; }

static void ep0_tx_stall(void)
{
    EP_TX_LEN(0) = 0;
    EP_TX_CTRL(0) = USBFS_EP_RES_STALL;
}

static void ep0_rx_ack(void)
{ EP_RX_CTRL(0) = USBFS_EP_RES_ACK; }

static void ep0_rx_stall(void)
{ EP_RX_CTRL(0) = USBFS_EP_RES_STALL; }

/**
 * @brief  Send data on EP0 (IN transaction).
 * @details Copies up to 64 bytes into the EP0 DMA buffer and enables TX
 *          with ACK.  Manages the data toggle automatically.
 * @param data  Pointer to the data to send (may be NULL for ZLP).
 * @param len   Number of bytes to send (truncated to 64 max).
 */
static void ep0_send(const uint8_t *data, uint16_t len)
{
    uint16_t send_len = (len > EP0_MAX_PACKET) ? EP0_MAX_PACKET : len;

    if (data && send_len)
        memcpy(ep0_buffer, data, send_len);

    EP_TX_LEN(0) = send_len;
    EP_TX_CTRL(0) = USBFS_EP_RES_ACK | (ep0_tog ? USBFS_EP_TOG : 0);
    ep0_tog = !ep0_tog;
}

/**
 * @brief  Prepare EP0 to receive data (OUT transaction).
 * @details Sets EP0 RX to ACK so the host can send data.
 */
static void ep0_recv(void)
{ EP_RX_CTRL(0) = USBFS_EP_RES_ACK; }

// ========== Standard request handlers ==========

/**
 * @brief  Handle standard USB requests (GET_DESCRIPTOR, SET_ADDRESS, etc.)
 * @param req  Pointer to the USB setup packet.
 * @param data Output: pointer to response data.
 * @param len  Output: length of response data.
 */
static void handle_std_request(const struct usb_setup_packet *req, const uint8_t **data, uint16_t *len)
{
    *data = NULL;
    *len = 0;

    switch (req->bRequest)
    {
        case USB_REQ_SET_ADDRESS: // SET_ADDRESS
            usb_dev_addr = req->wValue & 0x7F;
            // Address is applied after status stage
            break;

        case USB_REQ_GET_DESCRIPTOR: // GET_DESCRIPTOR
        {
            uint8_t desc_type = (req->wValue >> 8) & 0xFF;
            uint8_t desc_idx = req->wValue & 0xFF;

            if (desc_type == USB_DT_DEVICE)
            {
                *data = (const uint8_t *)&dev_desc;
                *len = sizeof(dev_desc);
            }
            else if (desc_type == USB_DT_CONFIG)
            {
                static __attribute__((aligned(4))) uint8_t cfg_buf[EP0_MAX_PACKET];
                uint8_t *p = cfg_buf;

                // Configuration descriptor
                struct usb_config_descriptor cfg = {.bLength = USB_DT_CONFIG_SIZE,
                                                    .bDescriptorType = USB_DT_CONFIG,
                                                    .wTotalLength = USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE +
                                                                    sizeof(struct usb_dfu_descriptor),
                                                    .bNumInterfaces = 1,
                                                    .bConfigurationValue = 1,
                                                    .iConfiguration = 0,
                                                    .bmAttributes = USB_CONFIG_SELF_POWERED,
                                                    .bMaxPower = 50};
                memcpy(p, &cfg, USB_DT_CONFIG_SIZE);
                p += USB_DT_CONFIG_SIZE;

                // Interface descriptor (DFU)
                struct usb_interface_descriptor ifc = {.bLength = USB_DT_INTERFACE_SIZE,
                                                       .bDescriptorType = USB_DT_INTERFACE,
                                                       .bInterfaceNumber = 0,
                                                       .bAlternateSetting = 0,
                                                       .bNumEndpoints = 0,
                                                       .bInterfaceClass = USB_DFU_CLASS,
                                                       .bInterfaceSubClass = USB_DFU_SUBCLASS,
                                                       .bInterfaceProtocol = USB_DFU_PROTOCOL,
                                                       .iInterface = STRIDX_DFUSE};
                memcpy(p, &ifc, USB_DT_INTERFACE_SIZE);
                p += USB_DT_INTERFACE_SIZE;

                // DFU functional descriptor
                memcpy(p, &dfu_func_desc, sizeof(dfu_func_desc));
                p += sizeof(dfu_func_desc);

                *data = cfg_buf;
                *len = (uint16_t)(p - cfg_buf);
            }
            else if (desc_type == USB_DT_STRING) // String
            {
                static __attribute__((aligned(4))) uint8_t sbuf[100];
                uint8_t idx = req->wValue & 0xFF;
                uint8_t *sp = sbuf;

                if (idx == 0) // LANGIDs
                {
                    sbuf[0] = USB_DT_STRING_HEADER + 2; // 2 bytes of language ID
                    sbuf[1] = USB_DT_STRING;
                    sbuf[2] = USB_LANGID_ENGLISH_US & 0xFF;
                    sbuf[3] = (USB_LANGID_ENGLISH_US >> 8) & 0xFF;
                    *len = USB_DT_STRING_HEADER + 2;
                }
                else
                {
                    const char *s = "";
                    switch (idx)
                    {
                        case STRIDX_MFR: s = DFU_STRING_MFR; break;
                        case STRIDX_PRODUCT: s = DFU_STRING_PRODUCT; break;
                        case STRIDX_SERIAL: s = DFU_STRING_SERIAL; break;
                        case STRIDX_DFUSE:
                        {
                            // ST DfuSe memory layout descriptor string.
                            // Format: @<Name>/<StartAddrHex>/<SegDef>
                            // Build:  @Internal Flash /0x0/00*YYYKg
                            // Where YYY = app size in KB.
                            // Built manually to avoid pulling sprintf / stdio syscalls.
                            static char dfuse_str[60];
                            snprintf(dfuse_str, 59, "@Internal Flash  /0x00000000/%d*%04dKa,%d*%04dKg",
                                     FLASH_BOOTLDR_SIZE_KB / DFU_SECTOR_SIZE_KB, DFU_SECTOR_SIZE_KB,
                                     FLASH_APP_SIZE_KB / DFU_SECTOR_SIZE_KB, DFU_SECTOR_SIZE_KB);
                            s = dfuse_str;
                        }
                        break;
                    }

                    *sp++ = USB_DT_STRING_HEADER + (uint8_t)strlen(s) * 2;
                    *sp++ = USB_DT_STRING; // string descriptor type
                    while (*s)
                    {
                        *sp++ = *s++;
                        *sp++ = 0;
                    }
                    *len = (uint16_t)(sp - sbuf);
                }
                *data = sbuf;
            }
            break;
        }

        case 0x08: // GET_CONFIGURATION
        {
            static uint8_t cfg_val = 1;
            *data = &cfg_val;
            *len = 1;
            break;
        }

        case 0x09: // SET_CONFIGURATION
            break; // nothing to do, we're in DFU mode

        default: break;
    }
}

// ========== Request routing ==========

extern int dfu_handle_request(const struct usb_setup_packet *req, const uint8_t **data, uint16_t *len);
extern void dfu_handle_data(const uint8_t *data, uint16_t len);

/**
 * @brief  Route a USB setup packet to the appropriate handler.
 * @details Standard requests (type 0x00) are handled locally; DFU class
 *          requests (type 0x20) are forwarded to @c dfu_handle_request().
 * @param req  Pointer to the USB setup packet.
 * @param data Output: pointer to response data.
 * @param len  Output: length of response data.
 */
static void route_request(const struct usb_setup_packet *req, const uint8_t **data, uint16_t *len)
{
    *data = NULL;
    *len = 0;

    uint8_t type = req->bmRequestType & 0x60;
    if (type == 0x00)
        handle_std_request(req, data, len);
    else if (type == 0x20)
        dfu_handle_request(req, data, len);
}

// ========== USB initialization ==========

/**
 * @brief  Initialise the USB peripheral and connect to the host.
 *
 * Resets the USB MAC/PHY, configures EP0, sets up endpoint control
 * registers, clears pending interrupts, and enables the device pull-up
 * to signal attachment.
 */
void usb_init(void)
{
    // Enable USB clock (peripheral clock already enabled by stage2_entry)
    // Delay for power-up and stabilization
    for (volatile int i = 0; i < 150000; i++)
        __asm__("nop");

    // Reset the USB MAC/PHY
    USB->BASE_CTRL = USBFS_CTRL_CLR_ALL | USBFS_CTRL_RESET_SIE;
    for (volatile int i = 0; i < 30000; i++)
        __asm__("nop");
    USB->BASE_CTRL = 0;

    // Initialize registers
    USB->BASE_CTRL = USBFS_CTRL_SYS_CTRL | USBFS_CTRL_INT_BUSY | USBFS_CTRL_DMA_EN;
    USB->UDEV_CTRL = USBFS_UDEV_CTRL_PD_DIS | USBFS_UDEV_CTRL_PORT_EN;
    USB->DEV_ADDR = 0x00;

    // Clear pending interrupts
    USB->INT_FG = 0xFF;

    // Enable interrupts we care about (polled: transfer, bus reset, suspend)
    USB->INT_EN = USBFS_INT_EN_BUS_RST | USBFS_INT_EN_TRANSFER | USBFS_INT_EN_SUSPEND;

    // Configure EP0
    EP_DMA(0) = (uint32_t)ep0_buffer;
    EP_TX_LEN(0) = 0;
    EP_TX_CTRL(0) = USBFS_EP_RES_NAK;
    EP_RX_CTRL(0) = USBFS_EP_RES_ACK;

    // All other endpoints: NAK, auto-toggle
    USB->UEP4_1_MOD = 0xCC;
    USB->UEP2_3_MOD = 0xCC;
    USB->UEP5_6_MOD = 0xCC;
    USB->UEP7_MOD = 0x0C;

    for (uint8_t ep = 1; ep < 8; ep++)
    {
        EP_DMA(ep) = 0;
        EP_TX_LEN(ep) = 0;
        EP_TX_CTRL(ep) = USBFS_EP_AUTO_TOG | USBFS_EP_RES_NAK;
        EP_RX_CTRL(ep) = USBFS_EP_AUTO_TOG | USBFS_EP_RES_NAK;
    }

    // Reset state
    ctrl_phase = CTRL_IDLE;
    ep0_tog = true;
    usb_dev_addr = 0;

    // Enable pull-up to connect to host
    USB->BASE_CTRL |= USBFS_CTRL_DEV_PUEN;
}

// ========== Poll loop ==========

/**
 * @brief  Poll the USB peripheral for pending events and handle them.
 *
 * Checks the interrupt flag register and processes:
 *   - Transfer complete (SETUP, IN, OUT)
 *   - Bus reset (reconfigures EP0 and resets state)
 *   - Suspend (clears the flag)
 *
 * USB does not use hardware interrupts — this function must be called
 * regularly from the main loop.
 */
void do_usb_poll(void)
{
    uint8_t int_fg = USB->INT_FG;

    if (int_fg & USBFS_INT_FG_TRANSFER)
    {
        // Read endpoint and token from INT_ST
        uint8_t int_st = USB->INT_ST;
        uint8_t ep = int_st & USBFS_INT_ST_ENDP_MASK;
        uint8_t token = USBFS_INT_ST_TOKEN(int_st);

        switch (token)
        {
            case PID_SETUP:
                // Setup packet received in EP0 DMA buffer
                // Clear any stall first
                EP_TX_CTRL(0) = USBFS_EP_RES_NAK;
                EP_RX_CTRL(0) = USBFS_EP_RES_ACK;

                // Parse the setup packet from the EP0 DMA buffer
                {
                    const struct usb_setup_packet *req = (const struct usb_setup_packet *)ep0_buffer;

                    ctrl_req_type = req->bmRequestType;
                    ctrl_request = req->bRequest;
                    ep0_tog = true; // reset toggle for control transfer

                    if (req->wLength == 0)
                    {
                        // No data stage — go directly to status
                        const uint8_t *d = NULL;
                        uint16_t l = 0;
                        route_request(req, &d, &l);

                        if (l == 0)
                        {
                            // Status: send ZLP
                            ctrl_phase = CTRL_STATUS;
                            ep0_send(NULL, 0);
                        }
                        else
                        {
                            // Send data, then status
                            ctrl_data_ptr = d;
                            ctrl_data_len = l;
                            ctrl_data_remaining = l;
                            ctrl_phase = CTRL_DATA_IN;
                            ep0_send(d, (l > EP0_MAX_PACKET) ? EP0_MAX_PACKET : l);
                        }
                    }
                    else if (req->bmRequestType & USB_DIR_IN)
                    {
                        // Device-to-host: send data
                        const uint8_t *d = NULL;
                        uint16_t l = 0;
                        route_request(req, &d, &l);

                        if (l > req->wLength)
                            l = req->wLength;

                        ctrl_data_ptr = d;
                        ctrl_data_len = l;
                        ctrl_data_remaining = l;
                        ctrl_phase = CTRL_DATA_IN;
                        ep0_send(d, (l > EP0_MAX_PACKET) ? EP0_MAX_PACKET : l);
                    }
                    else
                    {
                        // Host-to-device: receive data
                        const uint8_t *d = NULL;
                        uint16_t l = 0;
                        route_request(req, &d, &l);

                        ctrl_phase = CTRL_DATA_OUT;
                        ctrl_data_remaining = req->wLength;
                        dfu_rx_offset = 0;
                        ep0_recv();
                    }
                }
                break;

            case PID_IN:
                // IN transfer complete on EP
                if (ep == 0)
                {
                    if (ctrl_phase == CTRL_DATA_IN && ctrl_data_remaining > 0)
                    {
                        uint16_t sent =
                            (ctrl_data_remaining > EP0_MAX_PACKET) ? EP0_MAX_PACKET : (uint16_t)ctrl_data_remaining;
                        ctrl_data_remaining -= sent;

                        if (ctrl_data_remaining > 0)
                        {
                            // Send next chunk
                            uint16_t s =
                                (ctrl_data_remaining > EP0_MAX_PACKET) ? EP0_MAX_PACKET : (uint16_t)ctrl_data_remaining;
                            ep0_send(ctrl_data_ptr + (ctrl_data_len - ctrl_data_remaining), s);
                        }
                        else
                        {
                            // All data sent — status phase
                            ctrl_phase = CTRL_IDLE;

                            // Apply deferred set address
                            if (ctrl_request == USB_REQ_SET_ADDRESS)
                                USB->DEV_ADDR = usb_dev_addr;
                        }
                    }
                    else if (ctrl_phase == CTRL_STATUS)
                    {
                        // Status phase complete
                        ctrl_phase = CTRL_IDLE;

                        // Apply deferred set address
                        if (ctrl_request == USB_REQ_SET_ADDRESS)
                            USB->DEV_ADDR = usb_dev_addr;
                    }
                }
                break;

            case PID_OUT:
                // OUT data received
                if (ep == 0 && ctrl_phase == CTRL_DATA_OUT)
                {
                    uint16_t rx_len = USB->RX_LEN;

                    if (dfu_rx_offset + rx_len <= sizeof(dfu_rx_buffer))
                    {
                        memcpy(dfu_rx_buffer + dfu_rx_offset, ep0_buffer, rx_len);
                        dfu_rx_offset += rx_len;
                    }

                    ctrl_data_remaining -= rx_len;

                    if (ctrl_data_remaining == 0 || rx_len < EP0_MAX_PACKET)
                    {
                        // All data received — pass it to DFU handler
                        if (ctrl_request == DFU_DNLOAD && dfu_rx_offset > 0)
                        {
                            dfu_handle_data(dfu_rx_buffer, dfu_rx_offset);
                        }
                        else if (ctrl_request == DFU_DNLOAD && dfu_rx_offset == 0)
                        {
                            // zero-length DNLOAD (Manifest phase)
                            dfu_handle_data(dfu_rx_buffer, 0);
                        }

                        ctrl_phase = CTRL_IDLE;
                        ep0_send(NULL, 0); // send ZLP as ACK
                    }
                    else
                    {
                        ep0_recv(); // more data expected
                    }
                }
                break;

            default: break;
        }

        // Clear transfer interrupt flag
        USB->INT_FG = USBFS_INT_FG_TRANSFER;
    }
    else if (int_fg & USBFS_INT_FG_BUS_RST)
    {
        // USB bus reset — reconfigure EP0
        ep0_tog = true;
        USB->DEV_ADDR = 0x00;

        // Re-arm EP0 RX
        EP_RX_CTRL(0) = USBFS_EP_RES_ACK;

        ctrl_phase = CTRL_IDLE;

        // Clear bus reset flag
        USB->INT_FG = USBFS_INT_FG_BUS_RST;
    }
    else if (int_fg & USBFS_INT_FG_SUSPEND)
    {
        // Suspend — just clear the flag
        USB->INT_FG = USBFS_INT_FG_SUSPEND;
    }
}
