#pragma once

#define CFG_TUSB_MCU OPT_MCU_CH32V307 //OPT_MCU_STM32F1 OPT_MCU_CH32V307
// only one port
#define BOARD_DEVICE_RHPORT_NUM     0
#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_FULL_SPEED //OPT_MODE_FULL_SPEED
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE+BOARD_DEVICE_RHPORT_SPEED)

#define CFG_TUSB_OS OPT_OS_NONE

// CDC for the moment

#define CFG_TUD_CDC              0
#define CFG_TUD_MSC              0
#define CFG_TUD_HID              0
#define CFG_TUD_MIDI             0
#define CFG_TUD_VENDOR           0
#define CFG_TUD_DFU_RUNTIME      0
#define CFG_TUD_DFU              1
//
#define CFG_TUD_DFU_XFER_BUFSIZE 512

//
#define CFG_TUD_ENDPOINT0_SIZE    64
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))

#if 0 // Reduce buffers
// CDC FIFO size of TX and RX
  #define CFG_TUD_CDC_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
  #define CFG_TUD_CDC_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
  // CDC Endpoint transfer buffer size, more is faster
  #define CFG_TUD_CDC_EP_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)

#else
  #define CFG_TUD_CDC_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 128 : 64)
  #define CFG_TUD_CDC_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 128 : 64)
  // CDC Endpoint transfer buffer size, more is faster
  #define CFG_TUD_CDC_EP_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 128 : 64)
#endif
