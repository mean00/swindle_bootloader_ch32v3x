#include "esprit.h"

extern "C"
{
    extern const char _data_begin, _data_end;
    extern const char _bss_begin, _bss_end;
    extern const char _data_lma;

    int main();
    void __libc_init_array(void);
    void lnSystemTimerInit();

    __attribute__((noreturn)) void start_c()
    {
        volatile uint32_t *p = (volatile uint32_t *)0x20000000;
        volatile uint32_t r0 = p[0];
        volatile uint32_t r1 = p[1];
        // Copy .data from LMA to VMA
        __asm volatile("  mv t0, %0  \n" // src = _data_lma
                       "  mv t1, %1  \n" // dst = _data_begin
                       "  mv t2, %2  \n" // end = _data_end
                       "lp0:          \n"
                       "  lw t3, 0(t0)\n"
                       "  sw t3, 0(t1)\n"
                       "  addi t0,t0,4\n"
                       "  addi t1,t1,4\n"
                       "  bgt t2,t1,lp0\n"

                       // Zero .bss
                       "  mv t0, %3  \n" // begin = _bss_begin
                       "  mv t1, %4  \n" // end = _bss_end
                       "lp1:          \n"
                       "  sw x0, 0(t0)\n"
                       "  addi t0,t0,4\n"
                       "  bgt t1,t0,lp1\n"

                       ::"r"((uint32_t *)&_data_lma),
                       "r"((uint32_t *)&_data_begin), "r"((uint32_t *)&_data_end), "r"((uint32_t *)&_bss_begin),
                       "r"((uint32_t *)&_bss_end));

        //__libc_init_array();
        p[0] = r0; // restore "reboot to DFU signature"
        p[1] = r1;
        main();
        xAssert(0);
    }
}
