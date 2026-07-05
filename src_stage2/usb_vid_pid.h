/**
 * @file    usb_vid_pid.h
 * @brief   USB Vendor ID and Product ID for the SWINDLE DFU bootloader.
 *
 * @details Uses the generic/openmoko VID (0x1209) with a custom PID.
 *          These identifiers are used in the USB device descriptor to
 *          present the bootloader to the host operating system.
 *
 * @ingroup stage2
 */

#pragma once

/** @brief USB Vendor ID (VID) — 0x1209 (generic/openmoko). */
#define USB_VID 0x1d50 /* Generic */
/** @brief USB Product ID (PID) — 0x0001 (SWINDLE DFU). */
#define USB_PID 0x6030 /* SWINDLE DFU */
