/*



*/
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"

extern bool check_status();
extern void go_dfu();
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
        while (1)
        {
            lnDelay(100);
        }
    }
    // full setup
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
//--
