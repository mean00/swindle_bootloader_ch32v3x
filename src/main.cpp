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
    while (1)
    {
        lnDelay(100);
    }
    // Do we need to go dfu ?
    if (check_status() == false)
    {
        // nope, we can jump to the application
        while (1)
        {
            lnDelay(100);
        }
    }
    go_dfu();
}
/*

*/
void loop()
{
    xAssert(0);
}
//--
