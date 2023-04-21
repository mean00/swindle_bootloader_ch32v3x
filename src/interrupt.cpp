
#include "lnArduino.h"

#define INTERRUPT_STUBS(x)  extern "C" void x(void ) { deadEnd(0);}

 void dmaIrqHandler(int a, int b ) { deadEnd(0);}

INTERRUPT_STUBS(SW_Handler)
INTERRUPT_STUBS(USART0_IRQHandler)


/**
*/
volatile int sysTick=0;
extern "C" void   __attribute__((used)) __attribute__ ((interrupt ("IRQ"))) SysTick_Handler(void)
{
    sysTick++;
}

void xDelay(int a)
{
    int tail=sysTick+a;
    while(sysTick < tail)
    {
        __asm__("nop");
    }
}