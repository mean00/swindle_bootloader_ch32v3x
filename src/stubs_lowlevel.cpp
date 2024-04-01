/**
 * @file stubs_lowlevel.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnGPIO.h"
#include "pinout.h"


/**
 * @brief 
 * 
 */
extern "C" void vPortEnterCritical()
{
    deadEnd(0);
}
/**
 * @brief 
 * 
 */
extern "C" void vPortExitCritical()
{
    deadEnd(0);
}
/**
 * @brief 
 * 
 * @param periph 
 */
void resetMe(const Peripherals periph)
{
    lnPeripherals::reset(periph);
    lnPeripherals::enable(periph);
}
// EOF