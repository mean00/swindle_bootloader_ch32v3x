/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#include "MapleFreeRTOS1000.h"
#include "systemHelper.h"
#pragma once

#define xAutoMutex lnAutoMutex

void xDelay(uint32_t ms);
extern "C"
{
    void __attribute__((noreturn)) do_assert(const char *a);
}
#define xAssert(a)                                                                                                     \
    if (!(a))                                                                                                          \
    {                                                                                                                  \
        do_assert(#a);                                                                                                 \
    }
extern "C" uint32_t SystemCoreClock;
// EOF
