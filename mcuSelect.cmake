# We ony enable usb for arm and non small footprint
IF(NOT DEFINED LN_ARCH)

    # try without crystal...
    #SET(LN_USE_INTERNAL_CLOCK True        CACHE INTERNAL "")
    SET(LN_MCU_SPEED          144000000    CACHE INTERNAL "") # 144 Mhz

    SET(LN_ARCH "RISCV"                   CACHE INTERNAL "")
    SET(LN_MCU  "CH32V3x"                 CACHE INTERNAL "")
    SET(LN_SPEC               "picolibc"  CACHE INTERNAL "") # if not set we use nano
    SET(LN_BOOTLOADER_SIZE    0           CACHE INTERNAL "") # ALSO UPDATE FLASH_BOOTLDR_SIZE_KB
    SET(LN_MCU_EEPROM_SIZE    0           CACHE INTERNAL "")
    SET(LN_MCU_STATIC_RAM     8           CACHE INTERNAL "")

    #SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
    IF(USE_CH32v3x_PURE_RAM) # TOTAL is 128k!
        SET(LN_MCU_RAM_SIZE       64          CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE     64         CACHE INTERNAL "")
    ELSE() # *not* pure ram
        SET(LN_MCU_RAM_SIZE       64          CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE     256         CACHE INTERNAL "")                
    ENDIF()


ENDIF(NOT DEFINED LN_ARCH)
MESSAGE(STATUS "Architecture ${LN_ARCH}, MCU=${LN_MCU}")
