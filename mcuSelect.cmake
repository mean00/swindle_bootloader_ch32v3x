# We ony enable usb for arm and non small footprint
IF(NOT DEFINED LN_ARCH)

    # try without crystal...
    #SET(LN_USE_INTERNAL_CLOCK True        CACHE INTERNAL "")
    #SET(LN_MCU_SPEED          48000000    CACHE INTERNAL "") # 96 Mhz
    #SET(LN_MCU_SPEED           96000000    CACHE INTERNAL "") # 96 Mhz
    SET(LN_MCU_SPEED          144000000    CACHE INTERNAL "") # 144 Mhz

    SET(LN_ARCH "RISCV"                   CACHE INTERNAL "")
    SET(LN_MCU  "CH32V3x"                 CACHE INTERNAL "")
    SET(LN_MCU_RAM_SIZE       64          CACHE INTERNAL "")
    SET(LN_MCU_FLASH_SIZE     256         CACHE INTERNAL "")
    SET(LN_MCU_STATIC_RAM     32          CACHE INTERNAL "")
    SET(LN_SPEC               "picolibc"  CACHE INTERNAL "") # if not set we use nano
    SET(LN_BOOTLOADER_SIZE    0           CACHE INTERNAL "") # ALSO UPDATE FLASH_BOOTLDR_SIZE_KB
    SET(LN_MCU_EEPROM_SIZE    0           CACHE INTERNAL "")
    #SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
ENDIF(NOT DEFINED LN_ARCH)
MESSAGE(STATUS "Architecture ${LN_ARCH}, MCU=${LN_MCU}")
