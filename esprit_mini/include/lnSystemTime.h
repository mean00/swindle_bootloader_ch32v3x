#pragma once
#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif
void lnSystemTimerInit();
#ifdef __cplusplus
}
#endif
uint32_t lnGetUs();
uint64_t lnGetUs64();
void lnDelayUs(uint32_t wait);
void lnDelay(uint32_t wait);
#define lnDelayMs lnDelay
uint32_t lnGetMs();
