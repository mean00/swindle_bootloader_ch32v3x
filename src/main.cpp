

#include "lnArduino.h"
#include "lnGPIO.h"
#include "lnCpuID.h"

#define FORCE_DFU_IO PB2

extern bool check_fw();
extern bool rebooted_into_dfu();
extern void jumpIntoApp();
extern void dfu();

void lnRunTimeInit()
{

}

extern "C" void __attribute__ ((noinline))  xdeadEnd(int code)
{
    while(1)
    {
        __asm__("nop");
    }
}

extern "C" void vPortEnterCritical()
{
    deadEnd(0);
}

extern "C" void vPortExitCritical()
{
    deadEnd(0);
}
/**
*/
bool check_forced_dfu()
{
	lnPinMode(FORCE_DFU_IO,    lnINPUT_PULLUP);
	for(int i=0;i<10;i++) // wait  a bit
		__asm__("nop");
	if(!lnDigitalRead(FORCE_DFU_IO)) // "OK" Key pressed
		return true;
	return false;
}

/**

*/
bool bootloader()
{

	// Activate GPIO B for now
	lnPeripherals::enable(pGPIOB);
	lnPeripherals::enable(pGPIOC);
	lnPeripherals::enable(pAF);


   	int go_dfu=false;
#define NEXT_STEP(x) {if(!go_dfu) go_dfu|=(int)x;}	
	NEXT_STEP(rebooted_into_dfu);
	NEXT_STEP(check_forced_dfu);
	NEXT_STEP(!check_fw());

	if(go_dfu==false)
	{
		jumpIntoApp();
	}
	lnCpuID::identify();
	dfu();
    return false;
}

//--