
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.13)
MESSAGE(STATUS "================================")
MESSAGE(STATUS "Starting swindle_bootloader_ch32v3x_ (ch32)")
MESSAGE(STATUS "================================")

OPTION(DEBUG_BUILD "Enable verbose, larger code" OFF)
include(./build_default.cmake)

#SET(DEBUG_BUILD ON CACHE INTERNAL "")


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

# Target CH32V303
SET(USE_CH32v3x_USB_OTG     True CACHE INTERNAL "")
SET(USE_CH32V3x             True CACHE INTERNAL "")
SET(LN_ENABLE_USBD          True CACHE INTERNAL "")
SET(LN_ENABLE_I2C           FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI           FALSE CACHE INTERNAL "")


IF(DEBUG_BUILD)
  ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6060")
ELSE()
  ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6030")
ENDIF()



SET(MINI ${CMAKE_SOURCE_DIR}/lnArduino_mini)
SET(AF_FOLDER  ${CMAKE_SOURCE_DIR}/lnArduino)

include(./mcuSelect.cmake)
SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${AF_FOLDER}/cmake CACHE INTERNAL "")

SET(LN_ENABLE_USBD  TRUE  CACHE INTERNAL "")
SET(LN_ENABLE_I2C   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_UART  FALSE  CACHE INTERNAL "")
SET(LN_ENABLE_RUST  FALSE  CACHE INTERNAL "")

# tiny USB configuration
SET(LN_USB_NB_CDC 0) # 2 CDC interfaces
SET(LN_USB_NB_HID 0) # No HID
SET(LN_USB_DFU_RUNTIME 0 CACHE INTERNAL "")
SET(LN_USB_DFU 1 CACHE INTERNAL "")

SET(LN_USB_BUFFER_SIZE 256 CACHE INTERNAL "") # increase usb cdc buff


PROJECT(swindle_ch32_bootloader NONE) # the NONE is very important !

ENABLE_LANGUAGE(C CXX ASM) # this is important too!
# Generate version
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(${AF_FOLDER})

INCLUDE(${AF_FOLDER}/setup.cmake)
USE_LIBRARY(tinyUsb)

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


IF(DEBUG_BUILD)
  ADD_DEFINITIONS("-O0 -g3 -DDEBUG_BUILD")
  SET(TAG "_DEBUG" CACHE INTERNAL "")
  include_directories(embedded_printf)
ELSE()
  ADD_DEFINITIONS("-Oz")
ENDIF(DEBUG_BUILD)
#
#ADD_DEFINITIONS(-DconfigTOTAL_HEAP_SIZE=16000 -DLN_MCU_SPEED=48000000)
SET(SRCS
        src/main.cpp
        src/hash_check.cpp
        src/xxhash.c
        src/rebooted.cpp
        src/dfu.cpp
        src/usb_setup.cpp
        #src/driver_uart.cpp
        #src/driver_timer.cpp
        #src/driver_interrupt.cpp
        #lnArduino_mini/ch32_src/start.S
        )
IF(DEBUG_BUILD)
  SET(EXTRA             src/lnDebug_mini.cpp                   embedded_printf/printf.c        )
ELSE()
  SET(EXTRA           src/lnDebug_empty.cpp       )
ENDIF()


#ADD_SUBDIRECTORY(tinyUsb)

#target_link_libraries(tinyUsb miniArduino)

SET(T ${CMAKE_SOURCE_DIR}/tinyUsb/src/src)

include_directories( ${T}/src/device)
include_directories( ${T}/src/)
include_directories( ${T}/../)

GENERATE_GD32_FIRMWARE( swindle_bootloader_ch32v3x_ ${LNA} ${SRCS} ${EXTRA} )
#TARGET_LINK_LIBRARIES(swindle_bootloader_ch32v3x_ miniArduino tinyUsb)
#TARGET_LINK_OPTIONS(swindle_bootloader_ch32v3x_  PRIVATE "-T${CMAKE_BINARY_DIR}/linker_script.ld" )
#--
#POSTPROCESS( swindle_bootloader_ch32v3x_ )
#
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


set_target_properties(swindle_bootloader_ch32v3x_ PROPERTIES OUTPUT_NAME swindle_bootloader_ch32v3x_${COMPILER}${TAG} )

#
