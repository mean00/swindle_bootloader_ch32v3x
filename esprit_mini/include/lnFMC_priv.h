#pragma once

struct LN_FMCx
{
    uint32_t WS;     // 00 ACR
    uint32_t KEY;    // 04 KEYR
    uint32_t OBKEY;  // 08 OPTKEYR
    uint32_t STAT;   // 0C SR
    uint32_t CTL;    // 10 CR
    uint32_t ADDR;   // 14 AR
    uint32_t dummy1; // 18
    uint32_t OBSTAT; // 1C OBR
    uint32_t WP;     // 20 WRPR
};
typedef volatile LN_FMCx LN_FMC;

// STAT
#define LN_FMC_STAT_BUSY      (1 << 0)
#define LN_FMC_STAT_PG_ERR    (1 << 3)
#define LN_FMC_STAT_WP_ERR    (1 << 4)
#define LN_FMC_STAT_WP_ENDF   (1 << 5)

// CTL
#define LN_FMC_CTL_PG     (1 << 0)
#define LN_FMC_CTL_PER    (1 << 1)
#define LN_FMC_CTL_MER    (1 << 2)
#define LN_FMC_CTL_OBPG   (1 << 4)
#define LN_FMC_CTL_OBER   (1 << 5)
#define LN_FMC_CTL_START  (1 << 6)
#define LN_FMC_CTL_LK     (1 << 7)
#define LN_FMC_CTL_OBWEN  (1 << 9)
#define LN_FMC_CTL_ERRIE  (1 << 10)
#define LN_FMC_CTL_ENDIE  (1 << 12)

// CH32V3x-specific FMC extensions
#define LN_FMC_CTL_CH32_FASTUNLOCK  (1 << 15)
#define LN_FMC_CTL_CH32_FASTPROGRAM (1 << 16)
#define LN_FMC_CTL_CH32_FASTERASE   (1 << 17)
#define LN_FMC_CTL_CH32_FASTSTART   (1 << 21)
#define LN_FMC_CTL_CH32_EHMOD       (1 << 24)

// WS (wait state / ACR) register — latency values
#define LN_FMC_WS_LATENCY_MASK      (0x7)
#define LN_FMC_WS_LATENCY_0         (0)   // 0-48 MHz
#define LN_FMC_WS_LATENCY_1         (1)   // 48-72 MHz
#define LN_FMC_WS_LATENCY_2         (2)   // 72-144 MHz
