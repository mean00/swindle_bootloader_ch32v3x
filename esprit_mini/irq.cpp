#include "esprit.h"

// PFIC interrupt controller
typedef struct
{
    uint32_t ISR[4];     // 0x00
    uint32_t dummy0[4];  // 0x10
    uint32_t IPR[4];     // 0x20
    uint32_t dummy1[4];  // 0x30
    uint32_t ITHRESHOLD; // 0x40
    uint32_t dummy2;
    uint32_t CFGR;     // 0x48
    uint32_t GISR;     // 0x4C
    uint8_t VTFIDR[4]; // 0x50
    uint32_t dummy3[3];
    uint32_t VTFADDR[4]; // 0x60
    uint32_t dummy4[(0x100 - 0x70) / 4];
    uint32_t IENR[4]; // 0x100
    uint32_t dummy5[(0x180 - 0x110) / 4];
    uint32_t IRER[4]; // 0x180
    uint32_t dummy6[(0x200 - 0x190) / 4];
    uint32_t IPSR[4]; // 0x200
    uint32_t dummy7[(0x280 - 0x210) / 4];
    uint32_t IPRR[4]; // 0x280
    uint32_t dummy8[(0x300 - 0x290) / 4];
    uint32_t IACTR[4]; // 0x300
    uint32_t dummy9[(0x400 - 0x310) / 4];
    uint32_t IPRIOIR[64]; // 0x400
} CH32V3_INTERRUPTx;
typedef volatile CH32V3_INTERRUPTx CH32V3_INTERRUPT;

static CH32V3_INTERRUPT *pfic = (CH32V3_INTERRUPT *)LN_PFIC_ADR;

// Machine status register bit for global interrupt enable
#define MSTATUS_MIE (1 << 3)

extern "C" void lnNoInterrupt()
{
    // Clear MIE in mstatus (machine status register)
    __asm volatile("csrc mstatus, %0" : : "r"(MSTATUS_MIE) : "memory");
}

extern "C" void lnInterrupts()
{
    // Set MIE in mstatus
    __asm volatile("csrs mstatus, %0" : : "r"(MSTATUS_MIE) : "memory");
}

void lnEnableInterrupt(const LnIRQ &irq)
{
    int irq_num = (int)irq;
    pfic->IENR[irq_num >> 5] = 1 << (irq_num & 0x1F);
}

void lnDisableInterrupt(const LnIRQ &irq)
{
    int irq_num = (int)irq;
    pfic->IRER[irq_num >> 5] = 1 << (irq_num & 0x1F);
}

void lnIrqSetPriority(const LnIRQ &irq, int prio)
{
    int irq_num = (int)irq;
    int s = (irq_num & 3) * 8;
    int r = irq_num >> 2;
    uint32_t b = pfic->IPRIOIR[r];
    b &= ~(0xff << s);
    b |= (prio << 4) << s;
    pfic->IPRIOIR[r] = b;
}
