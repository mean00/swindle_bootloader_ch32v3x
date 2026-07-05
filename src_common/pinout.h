/**
 * @file    pinout.h
 * @brief   Common pin definitions shared by stage-1 and stage-2 bootloaders.
 *
 * @details Defines the LED and DFU-detection GPIO pins used during the
 *          bootloader sequence.  These pins are accessed via the lnArduino
 *          GPIO abstraction layer.
 *
 * @ingroup common
 */

#pragma once

/** @brief Bootloader activity indicator — connected to PB13 (active high). */
#define LED_PIN   PB13
/**
 * @brief  DFU mode detection pin — connected to PB7.
 * @note   Active low with an external pull-up resistor.
 *         Pulling this pin to GND at reset forces DFU mode.
 */
#define DFU_PIN   PB7
