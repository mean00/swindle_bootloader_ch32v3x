/*
 *  (C) 2022/2023 MEAN00 fixounet@free.fr
 *  See license file
 *  PFIC is configured with 2 nested level, 1 bit for preemption
 *  That means interrupt priority between 0..7 ignoring preemption
 *
 */

#include "lnArduino.h"
#include "lnRCU.h"
#include "lnIRQ.h"
#include "lnIRQ_riscv_priv_ch32v3x.h"
/**
 * 
 */
void PromoteIrqToFast(const LnIRQ &irq, int no);
struct CH32V3_INTERRUPTx
{
    uint32_t ISR[4];                // 0x00 Interrupt Enable Status Register
    uint32_t dummy0[4];             // 0x10
    uint32_t IPR[4];                // 0x20 Interrupt Pending Status Register  
    uint32_t dummy1[4];             // 0x30
    uint32_t ITHRESHOLD;            // 0x40 Interrupt priority Threshold configuration register
    uint32_t dummy2;
    uint32_t CFGR      ;            // 0x48 Interrupt Configuration Register
    uint32_t GISR      ;            // 0x4C Global Status Register
    uint8_t  VTFIDR[4]    ;            // 0x50 VTF Interrupt ID configuration Register vector to assign to that fast vector
    uint32_t dummy3[3]; 
    uint32_t VTFADDR[4]    ;        // 0x60 VTD Interrupt Address Register
    uint32_t dummy4[(0x100-0x70)/4]; 
    uint32_t IENR[4]    ;           // 0x100 Interrupt Enable Set Register
    uint32_t dummy5[(0x180-0x110)/4];
    uint32_t IRER[4]    ;           // 0x180 Interrupt Enable Reset Register
    uint32_t dummy6[(0x200-0x190)/4];
    uint32_t IPSR[4]    ;           // 0x200 Interrupt Pending Set Register
    uint32_t dummy7[(0x280-0x210)/4];
    uint32_t IPRR[4]    ;           // 0x280 Interrupt Pending Reset Register
    uint32_t dummy8[(0x300-0x290)/4];
    uint32_t IACTR[4]    ;          // 0x300 Interrupt Activation Register
    uint32_t dummy9[(0x400-0x310)/4];
    uint32_t IPRIOIR[64];          // 0x400 Priority(0..63)
};

typedef volatile CH32V3_INTERRUPTx CH32V3_INTERRUPT;

CH32V3_INTERRUPT *pfic = (CH32V3_INTERRUPT *)LN_PFIC_ADR;
void _enableDisable_direct(bool enableDisable, const int &irq_num);

// Attribute [LEVEL1:LEVEL0][SHV] : 
//      LEVEL: 
//              0:0=LEVEL, 
//              0:1=RISING EDGE, 
//              1:0: FALLINGEDGE   
//      SHV:
//              0 non vectored
//              1 vectored
//
#define X(y) (uint32_t )y

void  __attribute__ ((noinline)) unsupported()
{
    deadEnd(11);
}

void  __attribute__ ((noinline)) HardFault()
{
    deadEnd(12);
}


#define DECLARE_INTERRUPT(x) void x()         LN_INTERRUPT_TYPE;
extern "C"
{
DECLARE_INTERRUPT( SysTick_Handler)
DECLARE_INTERRUPT( Ecall_M_Mode_Handler)
DECLARE_INTERRUPT( Ecall_U_Mode_Handler)
DECLARE_INTERRUPT( SW_Handler)
DECLARE_INTERRUPT( USART0_IRQHandler)
DECLARE_INTERRUPT( OTG_FS_IRQHandler)


}

#define FAST_UNUSED   0xFF00
uint16_t fastInterrupt[4]={FAST_UNUSED, FAST_UNUSED, FAST_UNUSED, FAST_UNUSED};

static  uint32_t dynVecTable[103]  __attribute__((aligned(32)));
/*

*/
static void initIrqTable()
{
    for(int i=0;i<sizeof(dynVecTable)/sizeof(uint32_t);i++)
        dynVecTable[i]= X(unsupported);

    dynVecTable[3] =  X(HardFault);
    dynVecTable[12]=  X(SysTick_Handler);
    dynVecTable[14]=  X(SW_Handler);
    dynVecTable[53]=  X(USART0_IRQHandler);
    dynVecTable[83]=  X(OTG_FS_IRQHandler);    
}

    
/**

*/
void lnIrqSysInit()
{
   // Disable fast
    for(int i=0;i<4;i++)
    {
        pfic->VTFIDR[i]=0;
        pfic->VTFADDR[i]=0;
        fastInterrupt[i]=FAST_UNUSED;
    }
    // Set all priorities to DEFAULT_PRIO (assuming 2 nested levels)
    #define DEFAULT_PRIO 5
    const uint32_t prio32 = (DEFAULT_PRIO<<4) | (DEFAULT_PRIO<<(4+8)) | (DEFAULT_PRIO<<(4+16)) | (DEFAULT_PRIO<<(4+24));
    for(int i=0;i<64;i++)
    {
        pfic->IPRIOIR[i]=prio32;
    }
    
    initIrqTable();
    // allow fast path for these 2 interrupts
    PromoteIrqToFast(LN_IRQ_SYSTICK, 1);
    //PromoteIrqToFast(LN_IRQ_SW, 2);
    // Set these two to higher number = lower priority
    lnIrqSetPriority(LN_IRQ_SYSTICK,6);
    //lnIrqSetPriority(LN_IRQ_SW,7);    

    

    // relocate vector table
    // Initialise WCH enhance interrutp controller, 
    uint32_t syscr=(0*CH32_SYSCR_HWSTKEN) | CH32_SYSCR_INESTEN | CH32_SYSCR_MPTCFG_2NESTED | CH32_SYSCR_HWSTKOVEN  ;
 	asm volatile(                       
                    "mv t0, %1\n"
                    "csrw 0x804, t0\n"   // INTSYSCR                    
                    "li t0, 0x7800\n"
                    "csrs mstatus, t0\n" // Enable floating point and interrupt 
                    "mv t0, %0 \n"
                    "ori t0, t0, 3\n"    //      Use vectored mode + relocate vector table
                    "csrw mtvec, t0 \n"  //                    
                  :: "r"(dynVecTable),"r"(syscr)
                );
   
    return;
}
/**

*/
bool xPortIsInsideInterrupt()
{
    uint32_t gisr = pfic -> GISR;    
    return (gisr>>8) & 1; // under interrupt
}

/**
*/
static int lookupIrq(int irq)
{
    switch(irq)
    {
        case LN_IRQ_SYSTICK:    return 12;break;
        case LN_IRQ_OTG_FS:     return (67+16);break;
        default:
                xAssert(0);
    }
    return 0;
}
/**
*/
void _enableDisable_direct(bool enableDisable, const int &irq_num)
{    
    if(enableDisable)
    {
        pfic->IENR[irq_num >> 5] = 1<< (irq_num & 0x1f); // 32 bits per register
    }
    else
    {
        pfic->IRER[irq_num >> 5] = 1<< (irq_num & 0x1f);
    }
    // fence ?
}
/**


*/
void _enableDisable(bool enableDisable, const LnIRQ &irq)
{
    int irq_num = lookupIrq(irq); //_irqs[irq].irqNb;
    _enableDisable_direct(enableDisable,irq_num);
}
/**
*/


/**

*/
void PromoteIrqToFast(const LnIRQ &irq, int no)
{
    if(no<1 || no > 4)
    {
        xAssert(0);
    }
    no--; // between 0 and 3 now
    int irq_num = lookupIrq(irq);
    uint32_t adr=dynVecTable[irq_num];
    fastInterrupt[no]=irq;
    pfic->VTFIDR[no]=irq_num;
    pfic->VTFADDR[no]=adr | 1; // fast path enabled by default, bit0
}


/**
 * 
 * @param per
 */
void lnEnableInterrupt(const LnIRQ &irq)
{   
    _enableDisable(true,irq);   
}


/**
    Set priority between 0 and 15
*/
void lnIrqSetPriority_direct(const int &irq_num, int prio )
{
    int s=(irq_num & 3)*8;
    int r=irq_num >> 2;
    uint32_t b = pfic->IPRIOIR[r];
    b&=~(0xff<<s);
    b|=(prio<<4) << s;
    pfic->IPRIOIR[r]=b;
}
/**

*/
void lnIrqSetPriority(const LnIRQ &irq, int prio )
{    
    int irq_num = lookupIrq(irq); //_irqs[irq].irqNb;
    lnIrqSetPriority_direct(irq_num,prio);
}
/**
 * 
 * @param per
 */
void lnDisableInterrupt(const LnIRQ &irq)
{
     _enableDisable(false,irq);
}





extern "C" void __attribute__ ((noinline))  deadEnd(int code)
{
    // No interrrupt
    ENTER_CRITICAL();
    while(1)
    {
        // blink red light...
        asm volatile("nop");
        
    }
}        


#define WEAK_INTERRUPT(x) void  __attribute__((weak))  x() {     xAssert(0); }

WEAK_INTERRUPT(OTG_FS_IRQHandler)



void lnSoftSystemReset()
{
}


// EOF


