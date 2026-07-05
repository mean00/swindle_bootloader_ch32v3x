/**
 * @file    registers.h
 * @brief   Common memory-map and layout constants for the CH32V3x bootloader.
 *
 * @details Provides the flash/RAM address ranges and the application offset
 *          used by both stage-1 and stage-2.  The APP_ADDRESS macro depends
 *          on @c FLASH_BOOTLDR_SIZE_KB which is supplied by the build system
 *          via @c memory_config.h.in.
 *
 * @ingroup common
 */

#pragma once

/**
 * @brief  Start address of the application image in flash.
 * @note   Computed as @c FLASH_BOOTLDR_SIZE_KB * 1024 from the base of
 *         flash (0x00000000), i.e. right after the bootloader region.
 */
#define APP_ADDRESS (0x00000000 + (FLASH_BOOTLDR_SIZE_KB) * 1024)

/** @brief Base address of the internal flash (aliased to 0x00000000). */
#define FLASH_START_ADDR 0x00000000UL
/** @brief Base address of the internal SRAM. */
#define RAM_START_ADDR 0x20000000UL

/** @brief Address of the machine-mode timer (mtime) register. */
#define MTIME_ADDR (0xE000F000)
