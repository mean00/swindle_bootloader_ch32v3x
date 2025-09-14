#include "esprit.h"
#include "memory_config.h"

/*
    Check  fw is valid
*/
extern "C" uint32_t ch32_crc(uint32_t addr, uint32_t len_in_u32);

bool check_fw()
{
    const uint32_t start_addr = 0x00000000 + (FLASH_BOOTLDR_SIZE_KB * 1024);
    const uint32_t *const base_addr = (uint32_t *)start_addr;

    uint32_t imageSize = base_addr[1];
    uint32_t checksum = base_addr[2];

    // Check hash of app is correct
    if (imageSize > 256 * 1024)
    {
        return false; // absurb size
    }
    // valid but no hash, we accept that too
    if (imageSize == 0x1234 && checksum == 0x5678)
    {
        return true; // un hashed default value, we accept them
    }

    // uint32_t computed = XXH32(&(base_addr[3]), imageSize, 0x100);
    uint32_t computed = ch32_crc((uint32_t)&(base_addr[3]), imageSize >> 2);
    return (computed == checksum);
}

// EOF
