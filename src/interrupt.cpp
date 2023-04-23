
#include "lnArduino.h"

#define INTERRUPT_STUBS(x)  extern "C" void x(void ) { deadEnd(0);}

 void dmaIrqHandler(int a, int b ) { deadEnd(0);}

INTERRUPT_STUBS(SW_Handler)
INTERRUPT_STUBS(USART0_IRQHandler)
/**  */
typedef struct
{
    volatile uint32_t CTLR;
    volatile uint32_t SR;
    volatile uint64_t CNT;
    volatile uint64_t CMP;
}SysTick_Type;
#define SysTick         ((SysTick_Type *) 0xE000F000)


/**
*/
volatile int sysTick=0;
extern "C" void   LN_INTERRUPT_TYPE SysTick_Handler(void)
{
    sysTick++;
    SysTick->SR=0;

}
void     noInterrupts()
{

}
void     interrupts()
{
    
}
void EnableIrqs()
{
      __asm__(
            "csrr   t1, mstatus \t\n"
            "ori    t1 , t1, 0x8 \t\n"
            "csrw   mstatus ,t1 \t\n" 
             ::  ) ;
}

void setupSysTick()
{
    // setup sys tick
    SysTick->CTLR= 0;
    SysTick->SR  = 0;
    SysTick->CNT = 0;
    SysTick->CMP = LN_MCU_SPEED /1000; // 1ms tick
    SysTick->CTLR= 0xf;
}


void xDelay(int a)
{
    int tail=sysTick+a;
    while(sysTick < tail)
    {
        __asm__("nop");
    }
}

void delayMicroseconds(int us)
{
    int ms=1+(us/1000);
    xDelay(ms);
}