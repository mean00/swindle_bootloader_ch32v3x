/*



*/
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

extern bool check_status();
extern void go_dfu();
extern void DisableIrqs();
/**
 */
void jumpIntoApp()
{
    DisableIrqs();
#define JUMP                                                                                                           \
    "lui t0, 0x4\t\n"                                                                                                  \
    "jalr x0,0(t0)\n"
    __asm__(JUMP ::);
}

/**
    \brief main code
*/
void setup()
{
    // Minimal setup

    // Do we need to go dfu ?
    if (check_status() == false)
    {
        // nope, we can jump to the application
        jumpIntoApp();
    }
    // yes
    go_dfu();
}
/*

*/
void loop()
{
    xAssert(0);
}
/*
 */
extern "C" void vTaskDelete(TaskHandle_t x)
{
    xAssert(0);
}
/**
 *
 */
extern "C" int vsnprintf_(char *buffer, size_t count, const char *format, va_list va)
{
    return 0;
}

//--
