
include_directories( ${MCU}/include )
ADD_DEFINITIONS("-DIRQn_Type=uint32_t")
ADD_DEFINITIONS("-DARCH_FPU=1")
ADD_LIBRARY(miniArduino STATIC
${MCU}/src/lnCpuID_riscv_ch32v3x.cpp
${MCU}/src/lnFMC_impl.cpp
${MCU}/src/lnIRQ_riscv_ch32v3x.cpp
${MCU}/src/ch32v3x_interrupt_table.cpp
${MCU}/boards/ch32v3x/lnPinMapping.cpp
${MCU}/src/lnRCU_systemClock_riscv_ch32v3x.cpp
${MCU}/src/lnRCU_ch32v3x_usb.cpp
${MCU}/src/lnUSBD_ch32v3x.cpp
${MCU}/src/lnRunTime_riscv_ch32v3x.cpp
${MCU}/src/lnSystemTimer_riscv_ch32v3x.cpp

src/systemAlloc.cpp
src/systemHelper.cpp

src/lnGPIO.cpp
src/lnRCU.cpp
)

target_include_directories( miniArduino PUBLIC include ch32_include private_include)
