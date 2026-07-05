#pragma once
#include "esprit_macro.h"
#include "stdint.h"
#include "lnGPIO_pins.h"

enum lnGpioMode
{
    lnFLOATING = 0,
    lnINPUT_FLOATING = 1,
    lnINPUT_PULLUP = 2,
    lnINPUT_PULLDOWN = 3,
    lnOUTPUT = 4,
    lnOUTPUT_OPEN_DRAIN = 5,
    lnALTERNATE_PP,
    lnALTERNATE_OD,
    lnPWM,
    lnADC_MODE,
    lnDAC_MODE,
    lnUART,
    lnSPI_MODE,
    lnUART_Alt,
};

void lnPinMode(const lnPin pin, const lnGpioMode mode, const int speedInMhz = 0);
void lnDigitalWrite(const lnPin pin, bool value);
bool lnDigitalRead(const lnPin pin);
void lnDigitalToggle(const lnPin pin);
