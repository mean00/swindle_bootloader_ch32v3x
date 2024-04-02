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
extern void EnableIrqs();

extern void resetMe(const Peripherals periph);

extern void _enableDisable_direct(bool enableDisable, const int &irq_num);
extern void setupSysTick();
extern void systemReset();


extern void printC(const char *c);
extern void printCHex(const char *c, uint32_t val_in_hex);
extern "C" void SysTick_Stop(void) ;
extern "C" void clockDefault();
extern void uartDeinit();

bool flashErase(uint32_t adr);
bool flashWrite(uint32_t adr, const uint8_t *data, int size);
uint32_t lnGetMs();

void lnDisableInterrupt(const LnIRQ &irq);
// EOF

