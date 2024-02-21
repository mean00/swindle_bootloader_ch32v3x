ADD_LIBRARY(miniArduino STATIC
ch32_src/lnCpuID_riscv_ch32v3x.cpp
ch32_src/lnFMC_impl.cpp
ch32_src/lnIRQ_riscv_ch32v3x.cpp
ch32_src/lnPinMapping.cpp
ch32_src/lnRCU_ch32v3x_usb.cpp
ch32_src/lnRCU_systemClock_riscv_ch32v3x.cpp
ch32_src/lnRunTime_riscv_ch32v3x.cpp
ch32_src/lnSystemTimer_riscv_ch32v3x.cpp
ch32_src/lnUSBD_ch32v3x.cpp
src/systemAlloc.cpp
src/systemHelper.cpp

src/lnGPIO.cpp
src/lnRCU.cpp
)

target_include_directories( miniArduino PUBLIC include ch32_include private_include)
