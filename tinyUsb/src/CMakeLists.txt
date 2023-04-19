
# CFG_TUSB_OS == OPT_OS_FREERTOS

SET(TUSB ${CMAKE_CURRENT_SOURCE_DIR}/src/src)
SET(LNSRC ${CMAKE_CURRENT_SOURCE_DIR}/lnSrc)
include_directories( ${AF_FOLDER}/private_include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/private_include)
include_directories( ${TUSB})
include_directories( ${TUSB}/common)
include_directories( ${TUSB}/device)
include_directories( ${TUSB}/class/device/cdc)
include_directories( ${TUSB}/class/device/dfu)


SET(CH32V3x_FOLDER      ${CMAKE_CURRENT_SOURCE_DIR}/ch32v3x/)

SET(DRIVERS             ${CH32V3x_FOLDER}/dcd_usbfs.c  ${CH32V3x_FOLDER}/dcd_usbfs_platform.cpp    )
SET(LN_OPT_TUSB_MCU     OPT_MCU_CH32V307)
INCLUDE_DIRECTORIES(    ${CH32V3x_FOLDER}  )       
SET(LN_OPT_MODE         OPT_MODE_FULL_SPEED)              

SET(CLASS_FILES ${CLASS_FILES} ${TUSB}/class/dfu/dfu_device.c )

IF(FALSE)
    ADD_DEFINITIONS("-DCFG_TUSB_DEBUG=3")
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
