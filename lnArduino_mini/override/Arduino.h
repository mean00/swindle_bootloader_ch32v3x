#pragma once
#include "stdint.h"
#include "stddef.h"
extern uint64_t millis();
extern uint64_t micros();
extern void delay(int ms);
extern void delayMicroseconds(int us);
extern void noInterrupts();
extern void interrupts();

