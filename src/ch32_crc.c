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
    for (int i = len_in_u32; i > 0; i--)
    {
        uint32_t in = *(mem++);
#if 1
         uint32_t out = in;
#else
        uint32_t out =
            ((in >> 24) & 0xff) | (((in >> 16) & 0xff) << 8) | (((in >> 8) & 0xff) << 16) | ((in & 0xff) << 24);
#endif
        crc->data = out;
    }
    return (crc->data);
}

// EOF
