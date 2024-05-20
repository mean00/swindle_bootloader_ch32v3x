
# CFG_TUSB_OS == OPT_OS_FREERTOS

SET(TUSB                ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/libraries/tinyUsb/src/src)
SET(LNSRC               ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/libraries/tinyUsb/lnSrc)
SET(CH32V3x_FOLDER      ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/mcus/riscv_ch32v3x/tinyUsb/old)    
SET(CH32V3x_FOLDER_NEW  ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/mcus/riscv_ch32v3x/tinyUsb/new)    
#include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/libraries/tinyUsb/private_include )
#include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/lnArduino/libraries/tinyUsb/include )
#include( ${LN_MCU_FOLDER}/tinyUsb/usb_setup.cmake)
IF(USE_CH32v3x_HW_IRQ_STACK)
    ADD_DEFINITIONS("-DUSE_CH32v3x_HW_IRQ_STACK")
ENDIF()

SET(LN_OPT_MODE         OPT_MODE_FULL_SPEED)
SET(LN_OPT_TUSB_MCU     OPT_MCU_CH32V307)
    
IF(TRUE) # Old driver
    INCLUDE_DIRECTORIES(    ${CH32V3x_FOLDER}  )

    IF( USE_CH32v3x_USB_HS ) 
        SET(DRIVERS              
                        #${LNSRC}/lnUsbStack.cpp
                        ${TINY_FOLDER}/dcd_ch32_usbhs.c
                        ${CH32V3x_FOLDER}/lnUsbStubs.cpp
                        ${CH32V3x_FOLDER}/dcd_usbhs_platform.cpp
                )
    ELSE( )
        SET(DRIVERS     #${LNSRC}/lnUsbStack.cpp
                        #${CH32V3x_FOLDER}/dcd_usbfs.c  
                        ${CH32V3x_FOLDER}/dcd_ch32_usbfs.c
                        ${CH32V3x_FOLDER}/dcd_usbfs_platform.cpp
                        ${CH32V3x_FOLDER}/../lnUsbStubs.cpp
                )
    ENDIF()        

ELSE() # New driver
    SET(CH32V3x_FOLDER ${LN_MCU_FOLDER}/tinyUsb/new)
    INCLUDE_DIRECTORIES(    ${CH32V3x_FOLDER}  )

    IF( USE_CH32v3x_USB_HS ) 
        SET(DRIVERS              
                        #${LNSRC}/lnUsbStack.cpp
                        ${TINY_FOLDER}/dcd_ch32_usbhs.c
                        ${CH32V3x_FOLDER}/lnUsbStubs.cpp
                        ${CH32V3x_FOLDER}/dcd_usbhs_platform.cpp
                )
    ELSE( )
        SET(DRIVERS     #${LNSRC}/lnUsbStack.cpp                        
                        ${CH32V3x_FOLDER_NEW}/dcd_ch32_usbfs.c
                        ${CH32V3x_FOLDER_NEW}/dcd_usbfs_platform.cpp
                        ${CH32V3x_FOLDER_NEW}/../lnUsbStubs.cpp
                )
    ENDIF()        

ENDIF()

IF( DEFINED LN_USB_DFU)
    SET(LN_USB_DFU ${LN_USB_DFU} CACHE INTERNAL "")
ELSE()
    SET(LN_USB_DFU 0 CACHE INTERNAL "")
ENDIF()

#configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tusb_config.h.in ${CMAKE_BINARY_DIR}/tusb_config.h @ONLY)


SET(CLASS_FILES ${CLASS_FILES} ${TUSB}/class/dfu/dfu_device.c )



IF(TRUE)
    ADD_DEFINITIONS("-DCFG_TUSB_DEBUG=2")
    ADD_DEFINITIONS("-DCFG_TUSB_DEBUG_PRINTF=Logger_C")
ENDIF()


SET(SRCS  ${CLASS_FILES} 
          ${TUSB}/tusb.c
          ${TUSB}/device/usbd_control.c
          ${TUSB}/device/usbd.c
          ${TUSB}/common/tusb_fifo.c
          ${DRIVERS}
          )
add_library(tinyUsb STATIC ${SRCS})
target_include_directories(tinyUsb PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(tinyUsb PRIVATE ${AF_FOLDER}/private_include)
target_include_directories(tinyUsb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tinyUsb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tinyUsb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private_include)
target_include_directories(tinyUsb PUBLIC ${TUSB})
target_include_directories(tinyUsb PUBLIC ${TUSB}/common)
target_include_directories(tinyUsb PUBLIC ${TUSB}/device)
target_include_directories(tinyUsb PUBLIC ${TUSB}/class/device/cdc)
target_include_directories(tinyUsb PUBLIC ${TUSB}/class/device/dfu)

target_compile_definitions(tinyUsb PUBLIC  "${LN_TUSB_EXTRA_DEF}")
target_include_directories(tinyUsb PRIVATE "${LN_TUSB_EXTRA_IDIR}")
