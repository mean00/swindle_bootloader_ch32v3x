/**
 * @file    ch32_crc.c
 * @brief   CH32 hardware CRC32 computation for stage-1 bootloader
 *
 * @details Provides a minimal CRC32 calculation using the CH32's dedicated
 *          CRC32 peripheral (located at 0x40023000).  The peripheral is
 *          clock-gated via AHBPCENR[6] and is reset before each run.
 *
 *          This is used by stage-1 to validate the application image's
 *          checksum stored in the image header.
 *
 * @ingroup stage1
 */

#include "stdint.h"

/** @brief CH32 CRC32 peripheral base address. */
#define CH32_CRC32_ADDR 0x40023000
/** @brief Value written to the control register to reset the CRC unit. */
#define CH32_CRC32_CONTROL_RESET 1
/** @brief AHB peripheral clock enable register (for gating the CRC clock). */
#define AHBPCENR (*(uint32_t *)0x40021014UL)

/**
 * @brief  CRC32 peripheral register layout (CH32-specific).
 */
typedef struct
{
    uint32_t data;              /**< CRC data register (input/output). */
    uint32_t independant_data;  /**< Independent data register.        */
    uint32_t control;           /**< Control register (reset bit).     */
} CRC_IPx;

/** @brief Volatile alias for the CRC peripheral. */
typedef volatile CRC_IPx CRC_IP;

/**
 * @brief  Compute CRC32 over a region of 32-bit words using the CH32 hardware.
 *
 * Enables the CRC peripheral clock, resets the CRC unit, feeds each 32-bit
 * word of the region through the CRC data register, and returns the final
 * CRC32 value.
 *
 * @param addr       Start address of the data region (must be 4-byte aligned).
 * @param len_in_u32 Number of 32-bit words to process.
 * @return uint32_t  The computed CRC32 value.
 */
uint32_t ch32_crc(uint32_t addr, uint32_t len_in_u32)
{
    // Enable CRC clock
    AHBPCENR |= 1 << 6;

    CRC_IP *crc = (CRC_IP *)CH32_CRC32_ADDR;
    // Reset (writes 0xFFFFFFF by itself)
    crc->control = CH32_CRC32_CONTROL_RESET;

    uint32_t *mem = (uint32_t *)addr;
    uint32_t *lim = mem + len_in_u32;

    for (uint32_t *p = mem; p < lim; p++)
    {
        crc->data = *p;
    }

    return crc->data;
}
