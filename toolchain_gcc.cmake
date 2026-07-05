# =============================================================================#
# GCC Toolchain Setup
# =============================================================================#
message(STATUS "Using GCC toolchain")

# Hardware FPU or soft
if(USE_HW_FPU)
  SET(MARCH "rv32imafc_zicsr")
  SET(MABI  "ilp32f")
else()
  SET(MARCH "rv32imac_zicsr")
  SET(MABI  "ilp32")
endif()

# Toolchain executables
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_PATH ${PLATFORM_TOOLCHAIN_PATH})
set(TOOLCHAIN_PREFIX ${PLATFORM_PREFIX})
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${PLATFORM_TOOLCHAIN_SUFFIX})
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${PLATFORM_TOOLCHAIN_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}g++${PLATFORM_TOOLCHAIN_SUFFIX})
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objcopy${PLATFORM_TOOLCHAIN_SUFFIX})
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objdump${PLATFORM_TOOLCHAIN_SUFFIX})
set(CMAKE_SIZE         ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}size${PLATFORM_TOOLCHAIN_SUFFIX})

# Compiler flags
set(COMMON_FLAGS "-march=${MARCH} -mabi=${MABI} -msave-restore -g3 -Os")
set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections -fno-common")
set(COMMON_FLAGS "${COMMON_FLAGS} -fsigned-char -fmessage-length=0")
set(COMMON_FLAGS "${COMMON_FLAGS} -DLN_MCU=LN_MCU_CH32V3x -DLN_ARCH=LN_ARCH_RISCV")
set(COMMON_FLAGS "${COMMON_FLAGS} -DUSE_CH32v3x_HW_IRQ_STACK=1")

set(CMAKE_C_FLAGS   "${COMMON_FLAGS}" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "-march=${MARCH} -mabi=${MABI}" CACHE INTERNAL "")

# Linker flags
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf" CACHE INTERNAL "")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf" CACHE INTERNAL "")

set(LD_FLAGS_COMMON
    "-march=${MARCH}"
    "-mabi=${MABI}"
    "-nostartfiles"
    "-Wl,--gc-sections"
    "-Wl,--warn-common"
)

macro(generate_firmware TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_options(${TARGET_NAME} PRIVATE --specs=picolibc.specs -lc -lgcc)
endmacro()
