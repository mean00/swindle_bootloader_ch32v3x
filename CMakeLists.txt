
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.13)
MESSAGE(STATUS "================================")
MESSAGE(STATUS "Starting swindle_bootloader_ch32v3x_ (ch32)")
MESSAGE(STATUS "================================")

OPTION(DEBUG_BUILD "Enable verbose, larger code" OFF)
include(./build_default.cmake)

SET(LN_DISABLE_LOGGER_UART TRUE CACHE INTERNAL "")
SET(LN_DISABLE_FLOAT_PRINTF TRUE CACHE INTERNAL "")
SET(LN_DISABLE_DEFAULT_IRQ_TABLE TRUE CACHE INTERNAL "")
SET(LN_FREERTOS_CONFIG_DIR ${CMAKE_SOURCE_DIR}/src/freertos_config CACHE STRING "")
SET(LN_DISABLE_DEFAULT_INIT  TRUE CACHE INTERNAL "")
SET(LN_DISABLE_DEFAULT_DMA  TRUE CACHE INTERNAL "")
SET(LN_CUSTOM_TINYUSB_CONF TRUE CACHE INTERNAL "")
SET(LN_DISABLE_PRINTF TRUE CACHE INTERNAL "")

SET(LN_ENABLE_USBD  TRUE  CACHE INTERNAL "")
SET(LN_ENABLE_I2C   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_UART  FALSE  CACHE INTERNAL "")
SET(LN_ENABLE_RUST  FALSE  CACHE INTERNAL "")

# Target CH32V303
SET(USE_CH32v3x_USB_OTG     True CACHE INTERNAL "")
SET(USE_CH32V3x             True CACHE INTERNAL "")

# tiny USB configuration
SET(LN_USB_NB_CDC 0) # 2 CDC interfaces
SET(LN_USB_NB_HID 0) # No HID
SET(LN_USB_DFU_RUNTIME 0 CACHE INTERNAL "")
SET(LN_USB_DFU 1 CACHE INTERNAL "")

SET(LN_USB_BUFFER_SIZE  CACHE INTERNAL "") # increase usb cdc buff



MACRO(POSTPROCESS target)
  add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating bin file"
    )
  add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${target}>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Memory summary"
    )
ENDMACRO(POSTPROCESS target)


IF(DEBUG_BUILD)
  ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6160")
ELSE()
  ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6030")
ENDIF()

SET(AF_FOLDER  ${CMAKE_SOURCE_DIR}/lnArduino)
include(./mcuSelect.cmake)
include_directories(src/freertos_config)



SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${AF_FOLDER}/cmake CACHE INTERNAL "")



PROJECT(swindle_ch32_bootloader NONE) # the NONE is very important !
ENABLE_LANGUAGE(C CXX ASM) # this is important too!
# Generate version
include_directories(src/freertos_config)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(${AF_FOLDER})

INCLUDE(${AF_FOLDER}/setup.cmake)
USE_LIBRARY(tinyUsb)
include(${AF_FOLDER}/libraries/tinyUsb/tiny.cmake)

INCLUDE_DIRECTORIES(${AF_FOLDER}/private_include)

ADD_DEFINITIONS("-DUSE_CH32v3x_HW_IRQ_STACK=1")
ADD_DEFINITIONS("-DARCH_FPU=1")
# Update paths
IF(FALSE)
  IF(USE_CLANG)
    SET(CMAKE_TOOLCHAIN_FILE ${MINI}/toolchain_clang.cmake)
    configure_file( "${MINI}/clang_ram_offset_by_8.lld.in" "${CMAKE_BINARY_DIR}/linker_script.ld" @ONLY)
  ELSE(USE_CLANG)
    SET(CMAKE_TOOLCHAIN_FILE ${MINI}/toolchain.cmake)
    configure_file( "${MINI}/ld_ram_offset_by_8.lds.in" "${CMAKE_BINARY_DIR}/linker_script.ld" @ONLY)
  ENDIF(USE_CLANG)
ENDIF()

ADD_DEFINITIONS("-DLN_FREERTOS_HEAP_SIZE=(${LN_MCU_RAM_SIZE}-${LN_MCU_STATIC_RAM}-1)")


IF(DEBUG_BUILD)
  ADD_DEFINITIONS("-O1 -g3 -DDEBUG_BUILD")
  SET(TAG "_DEBUG" CACHE INTERNAL "")
ELSE()
  ADD_DEFINITIONS("-O1 -Os")
ENDIF(DEBUG_BUILD)
#
SET(SRCS
        src/main.cpp
        src/hash_check.cpp
        src/xxhash.c
        src/logger_override.cpp
        src/check_status.cpp
        src/go_dfu.cpp
#
        )

IF( LN_DISABLE_DEFAULT_IRQ_TABLE )
  SET( SRCS ${SRCS}  src/irq_override.cpp  )
ENDIF()

IF( LN_DISABLE_DEFAULT_INIT )
  SET( SRCS ${SRCS}  src/init_override.cpp )
ENDIF()

#src/usb_setup.cpp
#src/interrupt_override.cpp

GENERATE_GD32_FIRMWARE( swindle_bootloader_ch32v3x ${LNA} ${SRCS} ${EXTRA} )
#target_link_libraries( swindle_bootloader_ch32v3x tinyUsb lnArduino )



#
#
SET(TGT_NAME "CH32V3x")
#
#
#
MESSAGE(STATUS "=======")
MESSAGE(STATUS "Summary")
MESSAGE(STATUS "=======")


math(EXPR MCU_SPEED_M "${LN_MCU_SPEED}/1000000")

MESSAGE(STATUS "Built for ${LN_ARCH}/${LN_MCU} with ${LN_MCU_FLASH_SIZE} flash / ${LN_MCU_RAM_SIZE} ram")
MESSAGE(STATUS "          MCU running at ${MCU_SPEED_M} Mhz")

IF(USE_CLANG)
  SET(COMPILER "CLANG" CACHE INTERNAL "")
  MESSAGE(STATUS "          Compiler : CLANG")
ELSE()
  SET(COMPILER "GCC" CACHE INTERNAL "")
  MESSAGE(STATUS "          Compiler : GCC")
ENDIF()

IF(DEBUG_BUILD)
  MESSAGE(STATUS "          DEBUG BUILD")
ENDIF()

set_target_properties(swindle_bootloader_ch32v3x PROPERTIES OUTPUT_NAME swindle_bootloader_ch32v3x_${COMPILER}${TAG} )
#
