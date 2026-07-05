/**
 * @file    usb.h
 * @brief   CH32V3x USB OTG FS peripheral — register layout and DFU driver interface.
 *
 * @details Defines the WCH USBFS register map (byte-packed, base 0x50000000),
 *          protocol descriptor structs, DFU state machine enums, and the
 *          public API for the stage-2 USB DFU driver.
 *
 *          The driver operates in polled mode (no USB interrupts) and
 *          implements the USB DFU class for firmware download/upload.
 *
 * @ingroup stage2
 */

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// =========================================================================
// CH32V3x USB OTG FS peripheral — register layout and driver interface
// Base address: 0x50000000
// =========================================================================

// =========================================================================
// Standard USB constants
// =========================================================================

/** @brief USB descriptor type: device. */
#define USB_DT_DEVICE     1
/** @brief USB descriptor type: configuration. */
#define USB_DT_CONFIG     2
/** @brief USB descriptor type: string. */
#define USB_DT_STRING     3
/** @brief USB descriptor type: interface. */
#define USB_DT_INTERFACE  4
/** @brief USB descriptor type: endpoint. */
#define USB_DT_ENDPOINT   5

/** @brief USB standard request: GET_DESCRIPTOR. */
#define USB_REQ_GET_DESCRIPTOR  0x06
/** @brief USB standard request: SET_ADDRESS. */
#define USB_REQ_SET_ADDRESS     0x05

/** @brief Direction bit in bmRequestType: host-to-device (0) or device-to-host (1). */
#define USB_DIR_IN  0x80

/** @brief USB specification release 2.00 in BCD. */
#define USB_VERSION_2_0  0x0200
/** @brief USB specification release 1.10 in BCD. */
#define USB_VERSION_1_1  0x0110

/** @brief English (US) language ID for string descriptor 0. */
#define USB_LANGID_ENGLISH_US  0x0409

/** @brief Size of a device descriptor (18 bytes). */
#define USB_DT_DEVICE_SIZE     18
/** @brief Size of a configuration descriptor (9 bytes). */
#define USB_DT_CONFIG_SIZE      9
/** @brief Size of an interface descriptor (9 bytes). */
#define USB_DT_INTERFACE_SIZE   9
/** @brief Size of a string descriptor header (2 bytes: length + type). */
#define USB_DT_STRING_HEADER    2

/** @brief Configuration attribute: self-powered. */
#define USB_CONFIG_SELF_POWERED  0x80
/** @brief Max power per unit: 2 mA. */
#define USB_MAX_POWER_UNIT_mA    2

// =========================================================================
// DFU constants
// =========================================================================

/** @brief DFU interface class code (Application Specific). */
#define USB_DFU_CLASS     0xFE
/** @brief DFU interface subclass (DFU mode). */
#define USB_DFU_SUBCLASS  0x01
/** @brief DFU interface protocol (DFU mode). */
#define USB_DFU_PROTOCOL  0x02

/** @brief DFU functional descriptor type. */
#define DFU_FUNCTIONAL 0x21

/**
 * @brief  USBFS register layout (byte-packed per WCH design).
 *
 * All registers are 1, 2, or 4 bytes wide and packed without padding.
 * The structure mirrors the hardware register map at 0x50000000.
 */
typedef volatile struct
{
    uint8_t  BASE_CTRL;      /**< Base control (0x00).                     */
    uint8_t  UDEV_CTRL;      /**< Device control (0x01).                   */
    uint8_t  INT_EN;         /**< Interrupt enable (0x02).                 */
    uint8_t  DEV_ADDR;       /**< Device address (0x03).                   */
    uint8_t  Reserve0;       /**< Reserved (0x04).                         */
    uint8_t  MIS_ST;         /**< Miscellaneous status (0x05).             */
    uint8_t  INT_FG;         /**< Interrupt flag (0x06).                   */
    uint8_t  INT_ST;         /**< Interrupt status (0x07).                 */
    uint16_t RX_LEN;         /**< Received data length (0x08).             */
    uint16_t Reserve1;       /**< Reserved (0x0A).                         */
    uint8_t  UEP4_1_MOD;     /**< EP4/1 mode (0x0C).                       */
    uint8_t  UEP2_3_MOD;     /**< EP2/3 mode (0x0D).                       */
    uint8_t  UEP5_6_MOD;     /**< EP5/6 mode (0x0E).                       */
    uint8_t  UEP7_MOD;       /**< EP7 mode (0x0F).                         */
    uint32_t UEP0_DMA;       /**< EP0 DMA buffer address (0x10).           */
    uint32_t UEP1_DMA;       /**< EP1 DMA buffer address (0x14).           */
    uint32_t UEP2_DMA;       /**< EP2 DMA buffer address (0x18).           */
    uint32_t UEP3_DMA;       /**< EP3 DMA buffer address (0x1C).           */
    uint32_t UEP4_DMA;       /**< EP4 DMA buffer address (0x20).           */
    uint32_t UEP5_DMA;       /**< EP5 DMA buffer address (0x24).           */
    uint32_t UEP6_DMA;       /**< EP6 DMA buffer address (0x28).           */
    uint32_t UEP7_DMA;       /**< EP7 DMA buffer address (0x2C).           */
    uint16_t UEP0_TX_LEN;    /**< EP0 TX length (0x30).                    */
    uint8_t  UEP0_TX_CTRL;   /**< EP0 TX control (0x32).                   */
    uint8_t  UEP0_RX_CTRL;   /**< EP0 RX control (0x33).                   */
    uint16_t UEP1_TX_LEN;    /**< EP1 TX length (0x34).                    */
    uint8_t  UEP1_TX_CTRL;   /**< EP1 TX control (0x36).                   */
    uint8_t  UEP1_RX_CTRL;   /**< EP1 RX control (0x37).                   */
    uint16_t UEP2_TX_LEN;    /**< EP2 TX length (0x38).                    */
    uint8_t  UEP2_TX_CTRL;   /**< EP2 TX control (0x3A).                   */
    uint8_t  UEP2_RX_CTRL;   /**< EP2 RX control (0x3B).                   */
    uint16_t UEP3_TX_LEN;    /**< EP3 TX length (0x3C).                    */
    uint8_t  UEP3_TX_CTRL;   /**< EP3 TX control (0x3E).                   */
    uint8_t  UEP3_RX_CTRL;   /**< EP3 RX control (0x3F).                   */
    uint16_t UEP4_TX_LEN;    /**< EP4 TX length (0x40).                    */
    uint8_t  UEP4_TX_CTRL;   /**< EP4 TX control (0x42).                   */
    uint8_t  UEP4_RX_CTRL;   /**< EP4 RX control (0x43).                   */
    uint16_t UEP5_TX_LEN;    /**< EP5 TX length (0x44).                    */
    uint8_t  UEP5_TX_CTRL;   /**< EP5 TX control (0x46).                   */
    uint8_t  UEP5_RX_CTRL;   /**< EP5 RX control (0x47).                   */
    uint16_t UEP6_TX_LEN;    /**< EP6 TX length (0x48).                    */
    uint8_t  UEP6_TX_CTRL;   /**< EP6 TX control (0x4A).                   */
    uint8_t  UEP6_RX_CTRL;   /**< EP6 RX control (0x4B).                   */
    uint16_t UEP7_TX_LEN;    /**< EP7 TX length (0x4C).                    */
    uint8_t  UEP7_TX_CTRL;   /**< EP7 TX control (0x4E).                   */
    uint8_t  UEP7_RX_CTRL;   /**< EP7 RX control (0x4F).                   */
    uint32_t Reserve2;       /**< Reserved (0x50).                         */
    uint32_t OTG_CR;         /**< OTG control (0x54).                      */
    uint32_t OTG_SR;         /**< OTG status (0x58).                       */
} USBFS_TypeDef;

/** @brief USB OTG FS peripheral base address. */
#define USBOTG_BASE 0x50000000UL
/** @brief Pointer to the USBFS peripheral registers. */
#define USB ((USBFS_TypeDef *)USBOTG_BASE)

// ========== BASE_CTRL bit definitions ==========
/** @brief Enable DMA mode. */
#define USBFS_CTRL_DMA_EN       (1 << 0)
/** @brief Clear all USB registers. */
#define USBFS_CTRL_CLR_ALL      (1 << 1)
/** @brief Reset the SIE (serial interface engine). */
#define USBFS_CTRL_RESET_SIE    (1 << 2)
/** @brief Set interrupt busy. */
#define USBFS_CTRL_INT_BUSY     (1 << 3)
/** @brief System control. */
#define USBFS_CTRL_SYS_CTRL     (1 << 4)
/** @brief Device pull-up enable (DP/DM). */
#define USBFS_CTRL_DEV_PUEN     (1 << 5)
/** @brief Force low-speed mode. */
#define USBFS_CTRL_LOW_SPEED    (1 << 6)
/** @brief Host mode select. */
#define USBFS_CTRL_HOST_MODE    (1 << 7)

// ========== UDEV_CTRL bit definitions ==========
/** @brief Port enable. */
#define USBFS_UDEV_CTRL_PORT_EN  (1 << 0)
/** @brief General-purpose bit. */
#define USBFS_UDEV_CTRL_GP_BIT   (1 << 1)
/** @brief Low-speed enable. */
#define USBFS_UDEV_CTRL_LOW_SPEED (1 << 2)
/** @brief DM pin status. */
#define USBFS_UDEV_CTRL_DM_PIN   (1 << 4)
/** @brief DP pin status. */
#define USBFS_UDEV_CTRL_DP_PIN   (1 << 5)
/** @brief Pull-down disable. */
#define USBFS_UDEV_CTRL_PD_DIS   (1 << 7)

// ========== INT_EN (interrupt enable) bit definitions ==========
/** @brief Bus reset interrupt enable. */
#define USBFS_INT_EN_BUS_RST    (1 << 0)
/** @brief Detect interrupt enable. */
#define USBFS_INT_EN_DETECT     (1 << 0)
/** @brief Transfer interrupt enable. */
#define USBFS_INT_EN_TRANSFER   (1 << 1)
/** @brief Suspend interrupt enable. */
#define USBFS_INT_EN_SUSPEND    (1 << 2)
/** @brief Host SOF interrupt enable. */
#define USBFS_INT_EN_HST_SOF    (1 << 3)
/** @brief FIFO overflow interrupt enable. */
#define USBFS_INT_EN_FIFO_OV    (1 << 4)
/** @brief Device NAK interrupt enable. */
#define USBFS_INT_EN_DEV_NAK    (1 << 6)
/** @brief Device SOF interrupt enable. */
#define USBFS_INT_EN_DEV_SOF    (1 << 7)

// ========== INT_FG (interrupt flag) bit definitions ==========
/** @brief Bus reset flag. */
#define USBFS_INT_FG_BUS_RST    (1 << 0)
/** @brief Detect flag. */
#define USBFS_INT_FG_DETECT     (1 << 0)
/** @brief Transfer complete flag. */
#define USBFS_INT_FG_TRANSFER   (1 << 1)
/** @brief Suspend flag. */
#define USBFS_INT_FG_SUSPEND    (1 << 2)
/** @brief Host SOF flag. */
#define USBFS_INT_FG_HST_SOF    (1 << 3)
/** @brief FIFO overflow flag. */
#define USBFS_INT_FG_FIFO_OV    (1 << 4)
/** @brief SIE free flag. */
#define USBFS_INT_FG_SIE_FREE   (1 << 5)
/** @brief Toggle OK flag. */
#define USBFS_INT_FG_TOG_OK     (1 << 6)
/** @brief Is NAK flag. */
#define USBFS_INT_FG_IS_NAK     (1 << 7)

// ========== INT_ST (interrupt status) bit definitions ==========
/** @brief Endpoint number mask (bits 0-3). */
#define USBFS_INT_ST_ENDP_MASK  0x0F
/** @brief Token PID mask (bits 4-5). */
#define USBFS_INT_ST_TOKEN_MASK 0x30
/** @brief Token PID shift (bit 4). */
#define USBFS_INT_ST_TOKEN_SHIFT 4
/** @brief Extract token PID from interrupt status. */
#define USBFS_INT_ST_TOKEN(x)   (((x) >> 4) & 0x03)

/** @brief Token PID values (from INT_ST bits 4-5). */
enum usb_token_pid {
    PID_OUT   = 0, /**< OUT token.   */
    PID_SOF   = 1, /**< SOF token.   */
    PID_IN    = 2, /**< IN token.    */
    PID_SETUP = 3, /**< SETUP token. */
};

// ========== Endpoint control (TX_CTRL / RX_CTRL) bit definitions ==========
/** @brief Endpoint response mask. */
#define USBFS_EP_RES_MASK       (3 << 0)
/** @brief Endpoint response: ACK. */
#define USBFS_EP_RES_ACK        (0 << 0)
/** @brief Endpoint response: NYET. */
#define USBFS_EP_RES_NYET       (1 << 0)
/** @brief Endpoint response: NAK. */
#define USBFS_EP_RES_NAK        (2 << 0)
/** @brief Endpoint response: STALL. */
#define USBFS_EP_RES_STALL      (3 << 0)

/** @brief Data toggle bit. */
#define USBFS_EP_TOG            (1 << 2)
/** @brief Auto-toggle enable. */
#define USBFS_EP_AUTO_TOG       (1 << 3)

// ========== Endpoint register access macros ==========
/** @brief Access EP DMA address register. */
#define EP_DMA(ep)      (*((volatile uint32_t *)(&USB->UEP0_DMA) + (ep)))
/** @brief Access EP TX length register. */
#define EP_TX_LEN(ep)   (*((volatile uint16_t *)(&USB->UEP0_TX_LEN) + 2 * (ep)))
/** @brief Access EP TX control register. */
#define EP_TX_CTRL(ep)  (*((volatile uint8_t *)(&USB->UEP0_TX_CTRL) + 4 * (ep)))
/** @brief Access EP RX control register. */
#define EP_RX_CTRL(ep)  (*((volatile uint8_t *)(&USB->UEP0_RX_CTRL) + 4 * (ep)))


// =========================================================================
// USB Standard Descriptors
// =========================================================================

/** @brief DFU functional descriptor type code. */
#define DFU_FUNCTIONAL 0x21
/**
 * @brief  USB DFU functional descriptor structure.
 */
struct usb_dfu_descriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bmAttributes;
#define USB_DFU_CAN_DOWNLOAD      0x01
#define USB_DFU_CAN_UPLOAD        0x02
#define USB_DFU_MANIFEST_TOLERANT 0x04
#define USB_DFU_WILL_DETACH       0x08
    uint16_t wDetachTimeout;
    uint16_t wTransferSize;
    uint16_t bcdDFUVersion;
} __attribute__((packed));

/**
 * @brief  USB standard device descriptor structure.
 */
struct usb_device_descriptor
{
    uint8_t  bLength;            /**< Descriptor size in bytes.             */
    uint8_t  bDescriptorType;    /**< Descriptor type (device = 1).        */
    uint16_t bcdUSB;             /**< USB specification release (BCD).     */
    uint8_t  bDeviceClass;       /**< Device class code.                   */
    uint8_t  bDeviceSubClass;    /**< Device subclass code.                */
    uint8_t  bDeviceProtocol;    /**< Device protocol code.                */
    uint8_t  bMaxPacketSize0;    /**< Max packet size for EP0.             */
    uint16_t idVendor;           /**< Vendor ID (VID).                     */
    uint16_t idProduct;          /**< Product ID (PID).                    */
    uint16_t bcdDevice;          /**< Device release number (BCD).         */
    uint8_t  iManufacturer;      /**< Manufacturer string index.           */
    uint8_t  iProduct;           /**< Product string index.                */
    uint8_t  iSerialNumber;      /**< Serial number string index.          */
    uint8_t  bNumConfigurations; /**< Number of configurations.             */
} __attribute__((packed));

/**
 * @brief  USB standard configuration descriptor structure.
 */
struct usb_config_descriptor
{
    uint8_t  bLength;             /**< Descriptor size in bytes.            */
    uint8_t  bDescriptorType;     /**< Descriptor type (config = 2).       */
    uint16_t wTotalLength;        /**< Total length of data returned.      */
    uint8_t  bNumInterfaces;      /**< Number of interfaces.               */
    uint8_t  bConfigurationValue; /**< Configuration value to select.     */
    uint8_t  iConfiguration;      /**< Configuration string index.         */
    uint8_t  bmAttributes;        /**< Configuration characteristics.      */
    uint8_t  bMaxPower;           /**< Max power consumption (×2 mA).      */
} __attribute__((packed));

/**
 * @brief  USB standard interface descriptor structure.
 */
struct usb_interface_descriptor
{
    uint8_t  bLength;            /**< Descriptor size in bytes.            */
    uint8_t  bDescriptorType;    /**< Descriptor type (interface = 4).    */
    uint8_t  bInterfaceNumber;   /**< Interface number.                   */
    uint8_t  bAlternateSetting;  /**< Alternate setting number.            */
    uint8_t  bNumEndpoints;      /**< Number of endpoints used.            */
    uint8_t  bInterfaceClass;    /**< Interface class code.                */
    uint8_t  bInterfaceSubClass; /**< Interface subclass code.             */
    uint8_t  bInterfaceProtocol; /**< Interface protocol code.             */
    uint8_t  iInterface;         /**< Interface string index.              */
} __attribute__((packed));

/**
 * @brief  USB standard endpoint descriptor structure.
 */
struct usb_endpoint_descriptor
{
    uint8_t  bLength;          /**< Descriptor size in bytes.              */
    uint8_t  bDescriptorType;  /**< Descriptor type (endpoint = 5).       */
    uint8_t  bEndpointAddress; /**< Endpoint address (dir + number).      */
    uint8_t  bmAttributes;     /**< Endpoint attributes (transfer type).  */
    uint16_t wMaxPacketSize;   /**< Max packet size for this endpoint.    */
    uint8_t  bInterval;        /**< Polling interval (frames).            */
} __attribute__((packed));

/**
 * @brief  USB setup packet (control transfer request).
 */
struct usb_setup_packet
{
    uint8_t  bmRequestType; /**< Request characteristics (dir, type, rcpt). */
    uint8_t  bRequest;      /**< Specific request number.                   */
    uint16_t wValue;        /**< Value field (request-dependent).           */
    uint16_t wIndex;        /**< Index field (request-dependent).           */
    uint16_t wLength;       /**< Number of bytes in the data stage.         */
} __attribute__((packed));

// =========================================================================
// DFU Protocol definitions
// =========================================================================

/** @brief DFU class-specific request codes. */
enum dfu_req
{
    DFU_DETACH,     /**< Detach request.     */
    DFU_DNLOAD,     /**< Download request.   */
    DFU_UPLOAD,     /**< Upload request.     */
    DFU_GETSTATUS,  /**< Get status request. */
    DFU_CLRSTATUS,  /**< Clear status request. */
    DFU_GETSTATE,   /**< Get state request.  */
    DFU_ABORT,      /**< Abort request.      */
};

/** @brief DFU status codes returned in GETSTATUS responses. */
enum dfu_status
{
    DFU_STATUS_OK,                /**< No error condition.                     */
    DFU_STATUS_ERR_TARGET,        /**< Target (flash) error.                  */
    DFU_STATUS_ERR_FILE,          /**< File is not valid.                     */
    DFU_STATUS_ERR_WRITE,         /**< Write error.                           */
    DFU_STATUS_ERR_ERASE,         /**< Erase error.                           */
    DFU_STATUS_ERR_CHECK_ERASED,  /**< Erase check failed.                    */
    DFU_STATUS_ERR_PROG,          /**< Program error.                         */
    DFU_STATUS_ERR_VERIFY,        /**< Verify error.                          */
    DFU_STATUS_ERR_ADDRESS,       /**< Address out of range.                  */
    DFU_STATUS_ERR_NOTDONE,       /**< Operation not done.                    */
    DFU_STATUS_ERR_FIRMWARE,      /**< Firmware error.                        */
    DFU_STATUS_ERR_VENDOR,        /**< Vendor-specific error.                 */
    DFU_STATUS_ERR_USBR,          /**< USB reset (USBR) error.                */
    DFU_STATUS_ERR_POR,           /**< Power-on reset (POR) error.            */
    DFU_STATUS_ERR_UNKNOWN,       /**< Unknown error.                         */
    DFU_STATUS_ERR_STALLEDPKT,    /**< Stall packet received.                 */
};

/** @brief DFU state machine states. */
enum dfu_state
{
    STATE_APP_IDLE,             /**< Application idle.                  */
    STATE_APP_DETACH,           /**< Application detach requested.      */
    STATE_DFU_IDLE,             /**< DFU idle, waiting for commands.    */
    STATE_DFU_DNLOAD_SYNC,      /**< Download synchronisation.          */
    STATE_DFU_DNBUSY,           /**< Download busy (flash programming).  */
    STATE_DFU_DNLOAD_IDLE,      /**< Download idle, ready for next.     */
    STATE_DFU_MANIFEST_SYNC,    /**< Manifest synchronisation.          */
    STATE_DFU_MANIFEST,         /**< Manifest in progress.              */
    STATE_DFU_MANIFEST_WAIT_RESET, /**< Waiting for reset after manifest. */
    STATE_DFU_UPLOAD_IDLE,      /**< Upload idle.                       */
    STATE_DFU_ERROR,            /**< Error state.                       */
};

// =========================================================================
// Public API
// =========================================================================

/** @brief Maximum DFU transfer block size (4 KB). */
#define DFU_TRANSFER_SIZE 4096

/** @brief CH32V3x flash memory sector (page) size in bytes. */
#define FLASH_SECTOR_SIZE 256

/**
 * @brief  Initialise the USB peripheral and connect to the host.
 * @note   Must be called after clock/interrupt init and before @c do_usb_poll().
 */
void usb_init(void);

/**
 * @brief  Poll the USB peripheral for events and handle them.
 * @note   Should be called repeatedly in the main loop.  Handles bus reset,
 *         SETUP/IN/OUT transactions, and DFU class requests.
 */
void do_usb_poll(void);
