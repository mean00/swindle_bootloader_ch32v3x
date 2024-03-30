
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
SET(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

cmake_minimum_required(VERSION 3.13)
MESSAGE(STATUS "==========================================")
MESSAGE(STATUS "Starting swindle_bootloader_ch32v3x (ch32)")
MESSAGE(STATUS "==========================================")

OPTION(DEBUG_BUILD "Enable verbose, larger code" OFF)


SET(MINI        ${CMAKE_SOURCE_DIR}/lnArduino_mini CACHE INTERNAL "")
SET(AF_FOLDER   ${CMAKE_SOURCE_DIR}/lnArduino CACHE INTERNAL "")
SET(MCU         ${CMAKE_SOURCE_DIR}/lnArduino/mcus/riscv_ch32v3x CACHE INTERNAL "")

include(./build_default.cmake)

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

SET(LN_ENABLE_USBD          True  CACHE INTERNAL "")
SET(LN_ENABLE_I2C           FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI           FALSE CACHE INTERNAL "")


IF(DEBUG_BUILD)
    ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6060")
ELSE()
    ADD_DEFINITIONS("-DLN_ID_PRODUCT=0x6030")
ENDIF()

include(./mcuSelect.cmake)

include_directories(${MINI}/override)
# Update paths
IF(USE_CLANG)
    SET(xCMAKE_TOOLCHAIN_FILE ${MCU}/toolchain_clang.cmake)    
    configure_file( "${MINI}/clang_ram_offset_by_8.lld.in" "${CMAKE_BINARY_DIR}/linker_script.ld" @ONLY)
ELSE(USE_CLANG)
    SET(xCMAKE_TOOLCHAIN_FILE ${MCU}/toolchain.cmake)
    configure_file( "${MINI}/ld_ram_offset_by_8.lds.in" "${CMAKE_BINARY_DIR}/linker_script.ld" @ONLY)
ENDIF(USE_CLANG)

include(${xCMAKE_TOOLCHAIN_FILE})

PROJECT(swindle_bootloader_ch32v3x NONE) # the NONE is very important !
enable_language(C CXX ASM) # this is important too!

include_directories(${CMAKE_SOURCE_DIR}/override)

IF(DEBUG_BUILD)
    ADD_DEFINITIONS("-O0 -g3 -DDEBUG_BUILD")
    SET(TAG "_DEBUG" CACHE INTERNAL "")
ELSE()
    ADD_DEFINITIONS("-Oz")
ENDIF(DEBUG_BUILD)
#

include_directories(lnArduino/)
#include_directories(lnArduino/include)
include_directories(lnArduino/arduinoLayer/include)
include_directories( ${MCU}/include )
include_directories( ${MCU}/private_include )
include_directories( lnArduino/private_include )
include_directories( ${MINI} )
include_directories( ${MINI}/include )
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${CMAKE_SOURCE_DIR}/include)


ADD_DEFINITIONS(-DUSE_CH32v3x_HW_IRQ_STACK=1 -DconfigTOTAL_HEAP_SIZE=16000 -DLN_MCU_SPEED=${LN_MCU_SPEED} -DPRINTF_DISABLE_SUPPORT_FLOAT -isystem ${CMAKE_SOURCE_DIR}/override)
ADD_SUBDIRECTORY( ${MINI} )
SET(BL_SRCS 
        src/main.cpp    
        src/hash_check.cpp    
        src/xxhash.c
        src/rebooted.cpp
        src/dfu.cpp
        src/usb_setup.cpp
        src/driver_uart.cpp
        src/driver_timer.cpp
        src/driver_interrupt.cpp
        ${MCU}/start.S
        CACHE INTERNAL ""
        )
IF(DEBUG_BUILD)
    SET(EXTRA             src/lnDebug_mini.cpp                   lnArduino/embedded_printf/printf.c        )
ELSE()
    SET(EXTRA           src/lnDebug_empty.cpp       )
ENDIF()        



include(./tiny.cmake)

target_link_libraries(tinyUsb miniArduino)



ADD_EXECUTABLE(swindle_bootloader_ch32v3x ${BL_SRCS} ${EXTRA})
TARGET_LINK_LIBRARIES(swindle_bootloader_ch32v3x  tinyUsb )
TARGET_LINK_LIBRARIES(swindle_bootloader_ch32v3x  miniArduino)
TARGET_LINK_OPTIONS(swindle_bootloader_ch32v3x  PRIVATE "-T${CMAKE_BINARY_DIR}/linker_script.ld" )
#--
#
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


set_target_properties(swindle_bootloader_ch32v3x PROPERTIES OUTPUT_NAME swindle_bootloader_ch32v3x${COMPILER}${TAG} )
POSTPROCESS( swindle_bootloader_ch32v3x )

#
