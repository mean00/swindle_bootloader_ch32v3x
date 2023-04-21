
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.13)
MESSAGE(STATUS "======================")
MESSAGE(STATUS "Starting lnBMP_bootloader ")
MESSAGE(STATUS "======================")

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
SET(USE_CH32v3x_USB_OTG True CACHE INTERNAL "")
SET(USE_CH32V3x        True CACHE INTERNAL "")

SET(USE_CLANG       TRUE CACHE INTERNAL "")
#SET(LN_LTO "-flto"  CACHE INTERNAL "")

SET(LN_ENABLE_USBD  True CACHE INTERNAL "")
SET(LN_ENABLE_I2C   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI   FALSE CACHE INTERNAL "")

SET(MINI ${CMAKE_SOURCE_DIR}/lnArduino_mini)

include(./mcuSelect.cmake)
# Update paths
SET(CMAKE_TOOLCHAIN_FILE ${MINI}/toolchain_riscv_ch32v3x_clang.cmake)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${AF_FOLDER}/cmake CACHE INTERNAL "")

configure_file( "${MINI}/clang.lld.in" "${CMAKE_BINARY_DIR}/linker_script.ld" @ONLY)

PROJECT(lnBMP_bootloader NONE) # the NONE is very important !

enable_language(C CXX ASM) # this is important too!
#
include_directories( ${MINI} )
include_directories( ${MINI}/include )
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# tiny USB configuration
SET(LN_USB_NB_CDC 2) # 2 CDC interfaces
SET(LN_USB_NB_HID 0) # No HID
SET(LN_USB_DFU_RUNTIME 1 CACHE INTERNAL "")

ADD_DEFINITIONS(-DconfigTOTAL_HEAP_SIZE=16000 -DLN_MCU_SPEED=96000000)
ADD_DEFINITIONS(-O0 -g3)

SET(LNA ${MINI}/lnGPIO.cpp
        ${MINI}/start.S
        ${MINI}/lnRCU_ch32v3x.cpp
        ${MINI}/lnIRQ_riscv_ch32v3x.cpp
        ${MINI}/lnSystemInit.cpp
        ${MINI}/lnRCU.cpp
        ${MINI}/start.cpp
        ${MINI}/lnPinMapping.cpp
        ${MINI}/lnCpuID_riscv_ch32v3x.cpp
        ${MINI}/lnRCU_systemClock_riscv_ch32v3x.cpp 
        ${MINI}/lnIRQ_riscv_ch32v3x.cpp 
    )
    
SET(SRCS 
        src/main.cpp    
        src/hash_check.cpp    
        src/xxhash.c
        src/rebooted.cpp
        src/dfu.cpp
        src/interrupt.cpp
        )
ADD_SUBDIRECTORY(tinyUsb)

SET(T ${CMAKE_SOURCE_DIR}/tinyUsb/src/src)

include_directories( ${T}/src/device)
include_directories( ${T}/src/)
include_directories( ${T}/../)


ADD_EXECUTABLE(lnBMP_bootloader ${LNA} ${SRCS}  )
TARGET_LINK_LIBRARIES(lnBMP_bootloader tinyUsb)
TARGET_LINK_OPTIONS(lnBMP_bootloader  PRIVATE "-T${CMAKE_BINARY_DIR}/linker_script.ld" )
#--
POSTPROCESS( lnBMP_bootloader )
#--

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

set_target_properties(lnBMP_bootloader PROPERTIES OUTPUT_NAME lnBMP_BL_${LN_MCU} )


