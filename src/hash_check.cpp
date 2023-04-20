#include "lnArduino.h"
#include "xxhash.h"
#include "memory_config.h"

/*
    Check  fw is valid
*/

bool check_fw()
{
    const uint32_t start_addr = 0x00000000 + (FLASH_BOOTLDR_SIZE_KB*1024);
	const uint32_t * const base_addr = (uint32_t*)start_addr;
	
	uint32_t sig= base_addr[0];
	uint32_t imageSize = base_addr[6];
	uint32_t checksum=	base_addr[7];	
	
    if((sig >>20) != 0x200) // this does not look at a ram address
    {
        return false; 
    }

     // Check hash of app is correct
    if(imageSize>256*1024)
    {
        return false; // absurb size
    }
    // valid but no hash, we accept that too
    if(imageSize==0x1234 && checksum==0x5678)	 
    {
        return true; // un hashed default value, we accept them
    }

    uint32_t computed=	XXH32 (&(base_addr[4]),imageSize,0x100);
    return (computed==checksum);
}


// EOF