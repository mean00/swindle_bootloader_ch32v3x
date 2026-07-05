/**
 * @file    clocking.h
 * @brief   Clock configuration constants for CH32V3x at 144 MHz.
 *
 * @details This header defines the register aliases and bit constants
 *          needed to configure the system clock for 144 MHz operation
 *          using an 8 MHz external crystal × 18 PLL multiplication.
 *
 *          The RCU register aliases (@c RCC_CR, @c RCC_CFGR, etc.)
 *          are mapped to the lnArduino RCU peripheral for minimal-code
 *          clock setup in stage-2 (which does not run the full lnArduino
 *          initialisation sequence).
 *
 * @ingroup stage2
 */

#pragma once

// Clocking constants for CH32V3x at 144 MHz
// 8 MHz external crystal × 18 = 144 MHz

/** @brief Mask for the flash wait-state (latency) field. */
#define FLASH_ACR_LATENCY_MASK  0x7
/** @brief 2 wait states — sufficient for 72–144 MHz operation. */
#define FLASH_ACR_LATENCY_2WS   0x02  // 2 wait states for 72-144 MHz
/** @brief 3 wait states — required for >144 MHz operation. */
#define FLASH_ACR_LATENCY_3WS   0x03  // 3 wait states for >144 MHz

// RCU register aliases for stage2 minimal code
/** @brief Alias: RCU control register (CTL). */
#define RCC_CR     (arcu->CTL)
/** @brief Alias: RCU clock configuration register (CFG0). */
#define RCC_CFGR   (arcu->CFG0)
/** @brief Alias: RCU backup domain control register (BDCTL). */
#define RCC_BCDR   (arcu->BDCTL)
/** @brief Alias: RCU AHB peripheral clock enable register. */
#define RCC_AHBEN  (arcu->AHBEN)
/** @brief Alias: RCU APB2 peripheral clock enable register. */
#define RCC_APB2EN (arcu->APB2EN)

// Clock control bits
/** @brief HSE oscillator enable bit. */
#define RCC_CR_HSEON  (1 << 16)
/** @brief HSE oscillator ready flag. */
#define RCC_CR_HSERDY (1 << 17)
/** @brief PLL enable bit. */
#define RCC_CR_PLLON  (1 << 24)
/** @brief PLL lock (ready) flag. */
#define RCC_CR_PLLRDY (1 << 25)

/** @brief AHB prescaler: SYSCLK not divided. */
#define RCC_CFGR_HPRE_SYSCLK_NODIV  0x0
/** @brief APB1 prescaler: HCLK divided by 2. */
#define RCC_CFGR_PPRE1_HCLK_DIV2    0x4
/** @brief APB2 prescaler: HCLK not divided. */
#define RCC_CFGR_PPRE2_HCLK_NODIV   0x0

/** @brief System clock switch: PLL selected. */
#define RCC_CFGR_SW_SYSCLKSEL_PLLCLK 0x2
/** @brief Mask for the system clock switch bits. */
#define RCC_CFGR_SW_MASK             0x3
