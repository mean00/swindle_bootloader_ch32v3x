/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#include "systemHelper.h"
#pragma once

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
// EOF
