/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#include "lnArduino.h"
#include "lnRCU.h"
#include "lnRCU_priv.h"
#include "lnPeripheral_priv.h"
#include "lnCpuID.h"
LN_RCU *arcu=(LN_RCU *)LN_RCU_ADR;
/**
 */
struct RCU_Peripheral
{
    uint8_t                     periph;
    uint8_t                     AHB_APB; // 1=APB 1, 2=APB2,8=AHB
    uint32_t                    enable;
};

#define RCU_RESET   1
#define RCU_ENABLE  2
#define RCU_DISABLE 3

static const RCU_Peripheral mini_peripherals[]=
{           // PERIP          APB         BIT
    {        pUART0,         2,          LN_RCU_APB2_USART0EN},
    {        pGPIOA,         2,          LN_RCU_APB2_PAEN},
    {        pGPIOB,         2,          LN_RCU_APB2_PBEN},
    {        pGPIOC,         2,          LN_RCU_APB2_PCEN},
    {        pAF,            2,          LN_RCU_APB2_AFEN},
    {        pUSBFS_OTG_CH32v3x, 8,      LN_RCU_AHB_USBFSEN_OTG_CH32V3x},    
};
const RCU_Peripheral *getPeriph(int p)
{
    int index=0;
    switch(p)
    {
        case pUART0 : index=0;break;
        case pGPIOA : index=1;break;
        case pGPIOB : index=2;break;
        case pGPIOC : index=3;break;
        case pAF    : index=4;break;
        case pUSBFS_OTG_CH32v3x    : index=5;break;
        default:
                xAssert(0);

    }
    return &(mini_peripherals[index]);
}

// 1 : Reset
// 2: Enable
// 3: disable
static void _rcuAction(const Peripherals periph, int action)
{
    xAssert((int)periph<100);
    const RCU_Peripheral *o=getPeriph(periph);
    xAssert(o->periph==periph);
    xAssert(o->enable);
    switch(o->AHB_APB)
    {
        case 1: // APB1
            xAssert(0);
            break;
        case 2: // APB2
            switch(action)
            {
                case RCU_RESET:
                        arcu->APB2RST|= o->enable;
                        arcu->APB2RST&=~( o->enable); // not sure if it auto clears itself
                        break;
                case RCU_ENABLE: arcu->APB2EN |= o->enable;break;
                case RCU_DISABLE: arcu->APB2EN &=~o->enable;break;
                default : xAssert(0);break;
            }
            break;
        case 8: // AHB
            switch(action)
            {
                case RCU_RESET:
                        // We can only reset USB
                      //  if(periph==pUSB) xAssert(0);
                        // else just ignore
                        break;
                case RCU_ENABLE: arcu->AHBEN |= o->enable;break;
                case RCU_DISABLE: arcu->AHBEN &=~o->enable;break;
                default : xAssert(0);break;
            }

            break;
        default:
            xAssert(0);
    }
}

/**
 *
 * @param periph
 */
void lnPeripherals::reset(const Peripherals periph)
{
    _rcuAction(periph,RCU_RESET);
}
/**
 *
 * @param periph
 */
void lnPeripherals::enable(const Peripherals periph)
{
    _rcuAction(periph,RCU_ENABLE);
}
/**
 *
 * @param periph
 */
void lnPeripherals::disable(const Peripherals periph)
{
    _rcuAction(periph,RCU_DISABLE);
}
/**

*/
extern uint32_t _rcuClockApb1;
void lnPeripherals::enableUsb48Mhz()
{
  static bool usb48M=false;
  if(usb48M) return;
  usb48M=true;
  // careful, the usb clock must be off !
  int scaler=(2*lnPeripherals::getClock(pSYSCLOCK))/48000000;
  int x=0;
   // For riscv chip CH32V303/.. FIXME
    {
        switch(scaler)
        {
            case 2: x=0;break; // 48 Mhz
            case 4: x=1;break; // 96 Mhz
            case 6: x=2;break; // 144 Mhz
            default:
                                xAssert(0); // invalid sys clock
                break;
        }
        // enable internal pullup/down
        volatile uint32_t *exten=(volatile uint32_t *)0x40023800;
        volatile uint32_t reg=*exten;
        reg |= 2;
        *exten=reg;
    }
  
  uint32_t cfg0=arcu->CFG0;
  cfg0&=LN_RCU_CFG0_USBPSC_MASK;
  cfg0|=LN_RCU_CFG0_USBPSC(x);
  arcu->CFG0=cfg0;
}

// EOF
