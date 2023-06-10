#include "lnArduino.h"

void uartSend(const char *c);
extern "C" void uartSend_C(const char *c);
void uartPutChar(const char c);

void printC(const char *c)
{
    uartSend_C(c);
}
void hex4(int h)
{
    h=h&0xf;
    if(h>9)
    {
        h='a'+h-10;
        uartPutChar(h);
    }else
    {
        h='0'+h;
        uartPutChar(h);
    }
}
void hex16(uint16_t h)
{
    hex4(h>>12);
    hex4(h>>8);
    hex4(h>>4);
    hex4(h>>0);
}

void printCHex(const char *c, uint32_t hex)
{
    uartSend_C(c);uartSend_C(":0x");
    if(hex>(1<<16))
    {
        hex16(hex>>16);
    }
    hex16(hex & 0xffff);
    uartSend_C("\n");
}