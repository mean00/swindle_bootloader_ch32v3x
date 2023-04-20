#include "lnArduino.h"

// Points to the bottom of the stack, we should have 8 bytes free there
extern uint32_t __msp_init;
uint64_t *marker=(uint64_t *)0x0000000020000000 ; // marker is at the beginning


// Reboots the system into the bootloader, making sure
// it enters in DFU mode.
 void reboot_into_bootloader() {	
	*marker = 0xDEADBEEFCC00FFEEULL;
}

// Clears reboot information so we reboot in "normal" mode
void clear_reboot_flags() {	
	*marker = 0;
}

// Returns whether we were rebooted into DFU mode
bool rebooted_into_dfu() {	
	return (*marker == 0xDEADBEEFCC00FFEEULL);
}

/**
*/
void jumpIntoApp()
{
    deadEnd(0);
}