
#include "esprit.h"
#define MSTATUS_MIE (1 << 3)

extern "C" void deadEnd(int code)
{
    (void)code;
    __asm__("ebreak");
    __asm volatile("csrc mstatus, %0" : : "r"(MSTATUS_MIE) : "memory");
    while (1)
        __asm__("nop");
}
