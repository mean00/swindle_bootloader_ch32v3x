

#include "lnArduino.h"
#include "lnGPIO.h"


#define FORCE_DFU_IO PB2

bool check_fw();



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
bool bootloader()
{
   	int go_dfu=0;
	//go_dfu = rebooted_into_dfu();
	// Activate GPIO B for now
	lnPeripherals::enable(pGPIOB);
	lnPeripherals::enable(pGPIOC);
	lnPeripherals::enable(pAF);
	//lnExtiSWDOnly();

	lnPinMode(FORCE_DFU_IO,    lnINPUT_PULLUP);
	for(int i=0;i<10;i++) // wait  a bit
		__asm__("nop");
	if(!lnDigitalRead(FORCE_DFU_IO)) // "OK" Key pressed
		go_dfu|=1; 
	
	//RCC_CSR |= RCC_CSR_RMVF; // Clear reset flag

	//lnCpuID::identify();

	if(!go_dfu)
	{
		go_dfu=!check_fw();
	}
	//clear_reboot_flags();
    deadEnd(0);
    return false;
}

//--