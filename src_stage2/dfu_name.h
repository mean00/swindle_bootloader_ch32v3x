/**
 * @file    dfu_name.h
 * @brief   USB DFU descriptor string constants for the stage-2 bootloader.
 *
 * @details Defines the manufacturer, product, and serial number strings
 *          used in the USB DFU device descriptor.  These are referenced
 *          by the string descriptor indices (iManufacturer, iProduct)
 *          in the main USB descriptor tables.
 *
 * @ingroup stage2
 */

#pragma once

/** @brief Manufacturer string reported to the host via USB. */
#define DFU_STRING_MFR    "CH32V3x Bootloader"
/** @brief Product name string reported to the host via USB. */
#define DFU_STRING_PRODUCT "SWINDLE DFU"
/** @brief Serial number string reported to the host via USB. */
#define DFU_STRING_SERIAL "CH32V3x-001"
