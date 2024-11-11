
#pragma once
#define LED PC13
#define LED2 PA8
#define LED3 PB13
#define FORCED_DFU_PIN PB14
#define FORCED_DFU_PIN2 PB7

static const lnPin ledPins[] = {LED, LED2, LED3};
static const lnPin dfuPins[] = {FORCED_DFU_PIN, FORCED_DFU_PIN2};

#define NB_LEDS (sizeof(ledPins) / sizeof(lnPin))
#define NB_DFUS (sizeof(dfuPins) / sizeof(lnPin))
