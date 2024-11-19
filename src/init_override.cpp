
#include "lnArduino.h"
#include "lnPeripheral_priv.h"

__attribute__((section(".initial_heap"))) uint8_t ucHeap[configTOTAL_HEAP_SIZE];

extern void setup();
extern void loop();
extern "C" void _init();
void lnIrqSysInit();
void lnExtiSysInit();
void lnDmaSysInit();
void lnRunTimeInit();
void lnRunTimeInitPostPeripherals();
extern void go_dfu();
extern bool check_status();
// ??
void DisableIrqs()
{
}

/**
 *
 * @param
 */
void initTask(void *)
{
    LoggerInit();
    go_dfu();
    xAssert(0);
}
/**
 *
 * @return
 */

void resetMe(const Peripherals periph)
{
    lnPeripherals::reset(periph);
    lnPeripherals::enable(periph);
}
// dummy call to prevent the linker from removing it...
extern const uint32_t *lnGetFreeRTOSDebug();

int main()
{
    lnRunTimeInit();
    // Initialize system
    lnInitSystemClock();
    // ECLIC init
    lnIrqSysInit();
    // The LEDs are all on GPIO A
    resetMe(pGPIOA);
    resetMe(pGPIOB);
    resetMe(pGPIOC);
    lnRunTimeInitPostPeripherals();
    // Do we need to go dfu ?
    if (check_status() == false)
    {
        // nope, we can jump to the application
        DisableIrqs();
#define JUMP                                                                                                           \
    "lui t0, 0x4\t\n"                                                                                                  \
    "jalr x0,0(t0)\n"
        __asm__(JUMP ::);
    }
    // full setup

#ifndef LN_INITIAL_TASK_PRIORITY
#define LN_INITIAL_TASK_PRIORITY 2 //
#endif                             //

#ifndef LN_INITIAL_STACK_SIZE
#define LN_INITIAL_STACK_SIZE 1024 //
#endif                             //
    lnCreateTask(initTask, "entryTask", LN_INITIAL_STACK_SIZE, NULL, LN_INITIAL_TASK_PRIORITY);
    vTaskStartScheduler();
    deadEnd(25);
}
// EOF
