#include "esprit.h"

LN_RCU *arcu = (LN_RCU *)LN_RCU_ADR;

void lnPeripherals::enable(const Peripherals periph)
{
    switch (periph)
    {
        case pGPIOA: arcu->APB2EN |= LN_RCU_APB2_PAEN; break;
        case pGPIOB: arcu->APB2EN |= LN_RCU_APB2_PBEN; break;
        case pGPIOC: arcu->APB2EN |= LN_RCU_APB2_PCEN; break;
        case pGPIOD: arcu->APB2EN |= LN_RCU_APB2_PDEN; break;
        case pGPIOE: arcu->APB2EN |= LN_RCU_APB2_PEEN; break;
        case pAF: arcu->APB2EN |= LN_RCU_APB2_AFEN; break;
        case pUSB:
        case pUSBFS_OTG_CH32v3x: arcu->AHBEN |= LN_RCU_AHB_USBFSEN_OTG_CH32V3x; break;
        case pUSBHS_CH32v3x: arcu->AHBEN |= LN_RCU_AHB_USBHSEN_CH32V3x; break;
        case pDMA0: arcu->AHBEN |= LN_RCU_AHB_DMA0EN; break;
        case pDMA1: arcu->AHBEN |= LN_RCU_AHB_DMA1EN; break;
        default: break;
    }
}

void lnPeripherals::disable(const Peripherals periph)
{
    switch (periph)
    {
        case pGPIOA: arcu->APB2EN &= ~LN_RCU_APB2_PAEN; break;
        case pGPIOB: arcu->APB2EN &= ~LN_RCU_APB2_PBEN; break;
        case pGPIOC: arcu->APB2EN &= ~LN_RCU_APB2_PCEN; break;
        case pGPIOD: arcu->APB2EN &= ~LN_RCU_APB2_PDEN; break;
        case pGPIOE: arcu->APB2EN &= ~LN_RCU_APB2_PEEN; break;
        case pAF: arcu->APB2EN &= ~LN_RCU_APB2_AFEN; break;
        case pUSB:
        case pUSBFS_OTG_CH32v3x: arcu->AHBEN &= ~LN_RCU_AHB_USBFSEN_OTG_CH32V3x; break;
        case pUSBHS_CH32v3x: arcu->AHBEN &= ~LN_RCU_AHB_USBHSEN_CH32V3x; break;
        default: break;
    }
}

void lnPeripherals::reset(const Peripherals periph)
{
    // Minimal: not needed for bootloader operation
    (void)periph;
}
