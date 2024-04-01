#pragma once
#include "stdint.h"
extern bool check_fw();
extern bool rebooted_into_dfu();
extern void jumpIntoApp();
extern void dfu();
extern void clearRebootedIntoDfu();
extern void lnIrqSysInit();
extern void uartInit();
extern void DisableIrqs();
extern void resetMe(const Peripherals periph);

extern void printC(const char *c);
extern void printCHex(const char *c, uint32_t val_in_hex);



