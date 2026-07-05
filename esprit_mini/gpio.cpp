#include "esprit.h"

// GPIO register layout for CH32V3x
struct LN_GPIOx
{
    uint32_t CFGLR;  // 0x00
    uint32_t CFGHR;  // 0x04
    uint32_t INDR;   // 0x08
    uint32_t OUTDR;  // 0x0C
    uint32_t BSHR;   // 0x10
    uint32_t BCR;    // 0x14
    uint32_t LCKR;   // 0x18
};
typedef volatile LN_GPIOx LN_GPIO;

static LN_GPIO *gpioPort(int port)
{
    return (LN_GPIO *)(LN_GPIOA_ADR + port * 0x400);
}

/**
 *  Get port number (A=0, B=1, ...) and pin bit from lnPin enum value.
 */
static void decodePin(lnPin pin, int &port, int &bit)
{
    port = pin / 16;
    bit  = pin % 16;
}

void lnPinMode(const lnPin pin, const lnGpioMode mode, const int speedInMhz)
{
    (void)speedInMhz;
    int port, bit;
    decodePin(pin, port, bit);
    LN_GPIO *gp = gpioPort(port);

    // Enable the GPIO clock (handled by caller via lnPeripherals::enable)

    uint32_t ctl = (bit < 8) ? gp->CFGLR : gp->CFGHR;
    int shift = (bit < 8) ? (bit * 4) : ((bit - 8) * 4);

    ctl &= ~(0xF << shift);

    switch (mode)
    {
    case lnFLOATING:
    case lnINPUT_FLOATING:
        ctl |= (0x4 << shift); // 0100: floating input
        break;
    case lnINPUT_PULLUP:
    case lnINPUT_PULLDOWN:
        ctl |= (0x8 << shift); // 1000: pull up/down input
        break;
    case lnOUTPUT:
        ctl |= (0x1 << shift); // 0001: push-pull output, 10 MHz
        break;
    case lnOUTPUT_OPEN_DRAIN:
        ctl |= (0x6 << shift); // 0110: open-drain output
        break;
    default:
        break;
    }

    if (bit < 8)
        gp->CFGLR = ctl;
    else
        gp->CFGHR = ctl;

    // Handle pull-up/pull-down via BCR/BSHR
    if (mode == lnINPUT_PULLUP)
    {
        gp->BSHR = (1 << bit);       // set bit
    }
    else if (mode == lnINPUT_PULLDOWN)
    {
        gp->BCR = (1 << bit);        // clear bit
    }
}

void lnDigitalWrite(const lnPin pin, bool value)
{
    int port, bit;
    decodePin(pin, port, bit);
    LN_GPIO *gp = gpioPort(port);
    if (value)
        gp->BSHR = (1 << bit);
    else
        gp->BCR  = (1 << bit);
}

bool lnDigitalRead(const lnPin pin)
{
    int port, bit;
    decodePin(pin, port, bit);
    LN_GPIO *gp = gpioPort(port);
    return (gp->INDR >> bit) & 1;
}

void lnDigitalToggle(const lnPin pin)
{
    int port, bit;
    decodePin(pin, port, bit);
    LN_GPIO *gp = gpioPort(port);
    // Toggle: read OUTDR, XOR, write back via BSHR/BCR
    if (gp->OUTDR & (1 << bit))
        gp->BCR = (1 << bit);
    else
        gp->BSHR = (1 << bit);
}
