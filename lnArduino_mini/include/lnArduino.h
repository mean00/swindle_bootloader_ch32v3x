/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#pragma once

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#define LN_ARCH_UNKNOWN 0
#define LN_ARCH_RISCV 1
#define LN_ARCH_ARM 2

#if LN_ARCH == LN_ARCH_RISCV
#include "lnIRQ_riscv.h"
#define LN_FENCE() __asm volatile("fence.i")
#define LN_IOWRITE(adr, value)                                                                                         \
    {                                                                                                                  \
        *adr = value;                                                                                                  \
    }
#else
#if LN_ARCH == LN_ARCH_ARM
#define LN_FENCE()                                                                                                     \
    {                                                                                                                  \
    } // no need for fence on arm
#define LN_IOWRITE(adr, value)                                                                                         \
    {                                                                                                                  \
        *adr = value;                                                                                                  \
    }
#else
#error UNSUPPORTED ARCH
#endif
#endif

#include "Arduino.h"
#include "systemHelper.h"
#if 0
extern "C"
{
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

}

#include "lnFreeRTOS.h"
#endif
#include "lnDebug.h"
#include "lnGPIO.h"
#include "lnIRQ.h"
#include "lnPeripherals.h"
#include "lnRCU.h"

// #include "lnPrintf.h"
#include "lnSystemTime.h"

#define xAssert(x)                                                                                                     \
    if (!(x))                                                                                                          \
    deadEnd(1)

#define LN_ALIGN(x) __attribute__((aligned(x)))
#define LN_USED __attribute__((used))

extern "C" void free(void *a) __THROW;
extern "C" void *malloc(size_t size) __THROW __attribute_malloc__;
