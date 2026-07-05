#pragma once
#include "stdint.h"

enum LnIRQ : int
{
    LN_IRQ_NONE = 0,
    LN_IRQ_USBFS = 82,     // USBFS global interrupt (OTG FS)
    LN_IRQ_USBFS_WKUP = 42,
    LN_IRQ_SYSTICK = 12,
    LN_IRQ_SW = 14,
    LN_IRQ_LAST
};

extern "C" void lnNoInterrupt();
extern "C" void lnInterrupts();
void lnEnableInterrupt(const LnIRQ &irq);
void lnDisableInterrupt(const LnIRQ &irq);
void lnIrqSetPriority(const LnIRQ &irq, int prio);

#define ENTER_CRITICAL() lnNoInterrupt()
#define EXIT_CRITICAL()  lnInterrupts()
