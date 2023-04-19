
#include "lnArduino.h"

extern const char _data, _edata;
extern const char __bss_start, _end;
extern const char _data_lma;

extern "C" int main();
extern "C" void __libc_init_array(void);
extern "C" void   __attribute__((noreturn))  start_c()
{
    uint32_t srcAdr= *(uint32_t *)(&_data_lma);
    volatile uint32_t *src = (volatile uint32_t *)srcAdr;
    volatile uint32_t *dst = (volatile uint32_t*)&_data;
    volatile uint32_t *end = (volatile uint32_t*)&_edata;

    while (dst < end) 
    {
            *dst++ = *src++;
    }
    
    /* Zero .bss. */
    volatile uint32_t *zstart = (volatile uint32_t*)&__bss_start;
    volatile uint32_t *zend =   (volatile uint32_t*)&_end;
    while (zstart < zend) 
    {
        *zstart++ = 0;
    }
    __libc_init_array(); // call ctor before jumping in the code
    main();
    xAssert(0);  
}