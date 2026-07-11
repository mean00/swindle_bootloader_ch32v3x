/**
 * @file    flash.h
 * @brief   CH32V3x Flash FMC controller — register-level erase/write primitives.
 *
 * @details Provides a minimal set of inline functions for programming the
 *          internal flash memory of the CH32V3x.  Based on the proven
 *          implementation in @c esprit/mcus/riscv_ch32v3x/src/lnFMC_impl.cpp.
 *
 *          Supports:
 *            - 256-byte page erase (CH32 fast-erase mode)
 *            - 256-byte block write (CH32 fast-program mode with write cache)
 *            - Erased-page detection (0xFFFFFFFF or 0xE339E339 patterns)
 *            - Post-write verification via memcmp
 *
 * @warning All flash addresses passed to the FMC controller must use the
 *          0x08000000 offset (physical flash alias).
 *
 * @ingroup stage2
 */

#pragma once
#include <stdint.h>
#include <string.h>

/*
 * CH32V3x Flash FMC controller — register-level erase/write.
 * Based on esprit/mcus/riscv_ch32v3x/src/lnFMC_impl.cpp (working code).
 *
 * FMC base: 0x40022000
 * Erase: 256-byte pages (fast erase mode)
 * Write: 256-byte chunks via fast program mode
 * IMPORTANT: All flash addresses passed to the controller need +0x08000000 offset.
 */

/** @brief FMC peripheral base address. */
#define FLASH_SECTOR_SIZE 256
#define FMC_BASE 0x40022000

/**
 * @brief  FMC register layout.
 */
typedef volatile struct
{
    uint32_t WS;     /**< Wait-state register (0x00).    */
    uint32_t KEY;    /**< Unlock key register (0x04).    */
    uint32_t OBKEY;  /**< Option-byte key register (0x08). */
    uint32_t STAT;   /**< Status register (0x0C).        */
    uint32_t CTL;    /**< Control register (0x10).       */
    uint32_t ADDR;   /**< Address register (0x14).       */
    uint32_t dummy1; /**< Reserved (0x18).               */
    uint32_t OBSTAT; /**< Option-byte status (0x1C).     */
    uint32_t WP;     /**< Write-protect register (0x20). */
} FMC_TypeDef;

/** @brief Pointer to the FMC peripheral registers. */
#define FMC ((FMC_TypeDef *)FMC_BASE)

// STAT bits
/** @brief Flash busy flag (traditional erase/program). */
#define FMC_STAT_BUSY (1 << 0)
/** @brief Write-cache busy flag (fast program). */
#define FMC_STAT_WR_BUSY (1 << 1)
/** @brief Programming error flag. */
#define FMC_STAT_PG_ERR (1 << 3)
/** @brief Write-protect error flag. */
#define FMC_STAT_WP_ERR (1 << 4)
/** @brief End-of-operation flag. */
#define FMC_STAT_WP_ENDF (1 << 5)

// CTL bits
/** @brief Program (PG) enable bit. */
#define FMC_CTL_PG (1 << 0)
/** @brief Page erase (PER) enable bit. */
#define FMC_CTL_PER (1 << 1)
/** @brief Mass erase (MER) enable bit. */
#define FMC_CTL_MER (1 << 2)
/** @brief Option-byte program enable. */
#define FMC_CTL_OBPG (1 << 4)
/** @brief Option-byte erase enable. */
#define FMC_CTL_OBER (1 << 5)
/** @brief Start operation bit. */
#define FMC_CTL_START (1 << 6)
/** @brief Lock (LK) bit. */
#define FMC_CTL_LK (1 << 7)
/** @brief Option-byte write enable. */
#define FMC_CTL_OBWEN (1 << 9)
/** @brief Error interrupt enable. */
#define FMC_CTL_ERRIE (1 << 10)
/** @brief End-of-operation interrupt enable. */
#define FMC_CTL_ENDIE (1 << 12)

// CH32-specific fast-mode bits
/** @brief Fast unlock enable. */
#define FMC_CTL_FASTUNLOCK (1 << 15)
/** @brief Fast program mode enable. */
#define FMC_CTL_FASTPROGRAM (1 << 16)
/** @brief Fast erase mode enable. */
#define FMC_CTL_FASTERASE (1 << 17)
/** @brief Fast start (commit) bit. */
#define FMC_CTL_FASTSTART (1 << 21)
/** @brief Extended (high) mode enable. */
#define FMC_CTL_EHMOD (1 << 24)

/**
 * @brief  Wait until the FMC is no longer busy (traditional erase/program).
 */
static void fmc_wait_busy(void)
{
    while (FMC->STAT & FMC_STAT_BUSY)
        ;
}

/**
 * @brief  Wait until the write-cache is no longer busy (fast program).
 */
static void fmc_wait_wr_busy(void)
{
    while (FMC->STAT & FMC_STAT_WR_BUSY)
        ;
}

/**
 * @brief  Perform the CH32 fast-unlock sequence.
 * @note   Standard unlock + fast unlock (to offset 0x24) required before
 *         CH32-specific fast-program/erase operations.
 */
static void fmc_fast_unlock(void)
{
    // Standard unlock
    if (FMC->CTL & FMC_CTL_LK)
    {
        FMC->KEY = 0x45670123;
        FMC->KEY = 0xCDEF89AB;
    }

    // Fast unlock: write to offset 0x24 (CH32 specific)
    if (FMC->CTL & FMC_CTL_FASTUNLOCK)
    {
        volatile uint32_t *chf = (uint32_t *)FMC_BASE;
        chf[9] = 0x45670123; // offset 0x24
        chf[9] = 0xCDEF89AB;
    }
}

/**
 * @brief  Lock the flash controller (set the LOCK bit).
 */
static void fmc_lock(void)
{ FMC->CTL |= FMC_CTL_LK; }

/**
 * @brief  Disable interrupts globally via mstatus MIE bit.
 */
class AutoNoInterrupt
{
  public:
    AutoNoInterrupt()
    { __asm volatile("csrc mstatus, %0" : : "r"(1 << 3) : "memory"); }
    ~AutoNoInterrupt()
    { __asm volatile("csrs mstatus, %0" : : "r"(1 << 3) : "memory"); }
};

/**
 * @brief  Erase a 256-byte page using CH32 fast-erase mode.
 * @param addr  Physical flash address (e.g. 0x0000xxxx, NOT 0x0800xxxx).
 */
static bool fmc_erase_page(uint32_t addr)
{
    // CH32 FMC expects address in 0x08000000+ space
    uint32_t fmc_addr = addr + 0x08000000;

    AutoNoInterrupt noInt;
    fmc_fast_unlock();

    FMC->CTL |= FMC_CTL_FASTERASE;
    FMC->ADDR = fmc_addr;
    FMC->CTL |= FMC_CTL_START;
    fmc_wait_busy();

    uint32_t stat = FMC->STAT;
    FMC->STAT = FMC_STAT_WP_ENDF; // clear end-of-operation
    FMC->CTL &= ~FMC_CTL_FASTERASE;
    FMC->CTL &= ~FMC_CTL_START;

    if (stat & (FMC_STAT_PG_ERR | FMC_STAT_WP_ERR))
    {
        FMC->STAT = FMC_STAT_PG_ERR | FMC_STAT_WP_ERR;
        return false;
    }
    if (!(stat & FMC_STAT_WP_ENDF))
        return false;

    return true;
}

/**
 * @brief  Erase a region of flash using 256-byte page erases.
 * @param startAddress  Start address (must be 1 KB-aligned).
 * @param sizeInKBytes  Number of kilobytes to erase.
 */
static bool fmc_erase_range(uint32_t startAddress, int sizeInKBytes)
{
    for (int i = 0; i < sizeInKBytes * 4; i++)
    {
        if (!fmc_erase_page(startAddress))
            return false;
        startAddress += 256;
    }
    return true;
}

/**
 * @brief  Write 256 bytes to flash using CH32 fast-program mode.
 * @brief  Write 128 bytes to flash using CH32 fast-program mode.
 * @details The data is first loaded into a 64-word write cache, then
 *          committed to flash with the FASTSTART bit.
 * @param addr  Target flash address (without 0x08000000 offset).
 * @param data  Pointer to the 256-byte source buffer.
 * @return true on success, false on error.
 */
static bool fmc_write_block(uint32_t addr, const uint8_t *data)
{
    uint32_t fmc_addr = addr + 0x08000000;
    volatile uint32_t *prog = (volatile uint32_t *)fmc_addr;
    AutoNoInterrupt noInt;

    FMC->CTL |= FMC_CTL_FASTPROGRAM;
    fmc_wait_busy();
    fmc_wait_wr_busy();

    // Write 64 words (256 bytes) into the write cache
    if ((((uint32_t)data) & 3) == 0) // aligned
    {
        volatile uint32_t *rd = (volatile uint32_t *)data;
        for (int i = 0; i < 64; i++)
        {
            *prog = *rd;
            prog++;
            rd++;
            fmc_wait_wr_busy();
        }
    }
    else // unaligned
    {
        for (int i = 0; i < 64; i++)
        {
            uint32_t w = data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
            *prog = w;
            prog++;
            data += 4;
            fmc_wait_wr_busy();
        }
    }
    // Commit the cache
    FMC->CTL |= FMC_CTL_FASTSTART;
    fmc_wait_busy();

    // Check for errors
    uint32_t stat = FMC->STAT;
    FMC->CTL &= ~FMC_CTL_PG;
    FMC->CTL &= ~FMC_CTL_FASTPROGRAM;

    FMC->STAT = FMC_STAT_WP_ENDF;
    if (stat & (FMC_STAT_PG_ERR | FMC_STAT_WP_ERR))
    {
        FMC->STAT = FMC_STAT_PG_ERR | FMC_STAT_WP_ERR;
        return false;
    }
    if (!(stat & FMC_STAT_WP_ENDF))
        return false;

    return true;
}

/**
 * @brief  Write an arbitrary number of bytes to flash in 256-byte blocks.
 * @param startAddress  Target flash address (must be 4-byte aligned).
 * @param data          Source buffer pointer.
 * @param sizeInBytes   Number of bytes to write (must be a multiple of 256).
 * @return true on success, false on error.
 */
static bool fmc_write(uint32_t startAddress, const uint8_t *data, int sizeInBytes)
{
    fmc_fast_unlock();
    while (sizeInBytes > 0)
    {
        if (!fmc_write_block(startAddress, data))
            return false;
        data += 256;
        startAddress += 256;
        sizeInBytes -= 256;
    }
    return true;
}

/**
 * @brief  Check if a 256-byte flash page is erased.
 * @details The CH32 fast-erase fills erased pages with a 0xE339E339
 *          pattern instead of 0xFFFFFFFF.
 * @param addr  Address of the 256-byte page (must be 256-byte aligned, raw offset 0x00000000).
 * @return true if the page appears erased, false otherwise.
 */
static bool fmc_page_is_erased(uint32_t addr)
{
    // Ensure we are reading from physical flash, not potentially RAM alias
    const uint32_t *physical_page = (const uint32_t *)(addr | 0x08000000);
    for (int i = 0; i < 64; i++)
    {
        if (physical_page[i] != 0xFFFFFFFF && physical_page[i] != 0xE339E339)
            return false;
    }
    return true;
}

/**
 * @brief  Verify that a flash region matches the source data.
 * @param addr          Flash address to compare.
 * @param data          Source data pointer.
 * @param sizeInBytes   Number of bytes to compare.
 * @return true if the region matches, false otherwise.
 */
static bool fmc_verify(uint32_t addr, const uint8_t *data, int sizeInBytes)
{
    uint32_t physical_addr = addr | 0x08000000;
    const uint8_t *flash_ptr = (const uint8_t *)physical_addr;
    for (int i = 0; i < sizeInBytes; i++)
    {
        if (flash_ptr[i] != data[i])
            return false;
    }
    return true;
}

#undef FMC
