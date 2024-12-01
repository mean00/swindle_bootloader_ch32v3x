#include "stdint.h"

#define CH32_CRC32_ADDR 0x40023000
#define CH32_CRC32_CONTROL_RESET 1
#define AHBPCENR (*(uint32_t *)0x40021014UL)
typedef struct
{
    uint32_t data;
    uint32_t independant_data;
    uint32_t control;
} CRC_IPx;

typedef volatile CRC_IPx CRC_IP;

/**
 *
 *  __attribute__((naked))
 */
uint32_t ch32_crc(uint32_t addr, uint32_t len_in_u32)
{
    // Enable  CRC clock
    AHBPCENR |= 1 << 6;
    //
    CRC_IP *crc = (CRC_IP *)CH32_CRC32_ADDR;
    // reset writes 0xFFFFFFF by itself
    crc->control = CH32_CRC32_CONTROL_RESET;
    // crc->data = init;
    uint32_t *mem = (uint32_t *)addr;
    uint32_t *lim = mem;
    lim += len_in_u32;
    for (uint32_t *p = mem; p < lim; p++)
    {
        crc->data = *p;
    }
    return (crc->data);
}

// EOF
