#pragma once

#include "esprit_macro.h"
#include "stdint.h"
#include "string.h"

#define LN_ARCH_RISCV 1
#define LN_ARCH_ARM   2
#define LN_ARCH        LN_ARCH_RISCV
#define LN_MCU_CH32V3x 1
#define LN_MCU         LN_MCU_CH32V3x

#define LN_FENCE()    __asm volatile("fence.i")
#define LN_IOWRITE(adr, value) { *adr = value; }

#define LN_ALIGN(x)   __attribute__((aligned(x)))
#define LN_USED       __attribute__((used))

extern "C" void lnNoInterrupt();
extern "C" void lnInterrupts();

#include "lnAssert.h"
#include "lnPeripheral_priv.h"
#include "lnGPIO.h"
#include "lnPeripherals.h"
#include "lnRCU.h"
#include "lnRCU_priv.h"
#include "lnFMC_priv.h"
#include "lnSystemTime.h"
#include "lnIRQ.h"

extern uint32_t SystemCoreClock;
