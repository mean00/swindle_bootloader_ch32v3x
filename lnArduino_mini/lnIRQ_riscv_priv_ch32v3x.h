/*
 *  (C) 2022 MEAN00 fixounet@free.fr
 *  See license file
 */
#pragma once
#include "lnIRQ.h"
#include "lnPeripheral_priv.h"

#define CH32_SYSCR_HWSTKEN (1 << 0)        // Hardware stack enabled
#define CH32_SYSCR_INESTEN (1 << 1)        // Interrupt nesting enabled
#define CH32_SYSCR_MPTCFG_2NESTED (1 << 2) //
#define CH32_SYSCR_MPTCFG_8NESTED (3 << 2) //
#define CH32_SYSCR_HWSTKOVEN (1 << 4)      // Continue after hw stack overflow
#define CH32_SYSCR_GIHWSTKNEN (1 << 5)     // Temporarily disable interrupts & hw stack
