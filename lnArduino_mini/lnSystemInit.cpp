
#include "lnArduino.h"
#include "lnPeripheral_priv.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE]  __attribute__((aligned(32)));;

extern void setup();
extern void loop();
extern "C" void _init();
void lnIrqSysInit();
void lnExtiSysInit();
void lnDmaSysInit();
void lnRunTimeInit();
void lnRunTimeInitPostPeripherals();

// dummy call to prevent the linker from removing it...
extern const uint32_t *lnGetFreeRTOSDebug();

extern void bootloader(void);

int main()
{
    // move stack to the end of ucheap
    uint32_t stack=(uint32_t)(&ucHeap[configTOTAL_HEAP_SIZE-4]) & (~3);
    asm volatile(                       
                    "mv sp, %0\n"
                  :: "r"(stack)
                );


    lnRunTimeInit();
    bootloader();
}
// EOF
