# =============================================================================#
# Clang Toolchain Setup
# =============================================================================#
message(STATUS "Using Clang toolchain")

if(NOT PLATFORM_TOOLCHAIN_PATH)
  MESSAGE(FATAL_ERROR "PLATFORM_TOOLCHAIN_PATH is not defined in platformConfig.cmake !!")
endif()

list(APPEND CMAKE_SYSTEM_PREFIX_PATH "${PLATFORM_TOOLCHAIN_PATH}")

# Setup toolchain for cross compilation
set(CMAKE_SYSTEM_NAME Generic           CACHE INTERNAL "")
set(CMAKE_C_COMPILER_ID   "Clang"       CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_ID "Clang"       CACHE INTERNAL "")
set(CMAKE_C_COMPILER_WORKS      TRUE    CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS    TRUE    CACHE INTERNAL "")

# Toolchain paths
set(CMAKE_C_COMPILER    ${PLATFORM_CLANG_PATH}/clang${PLATFORM_CLANG_VERSION}${TOOLCHAIN_SUFFIX}    CACHE PATH "" FORCE)
set(CMAKE_ASM_COMPILER  ${PLATFORM_CLANG_PATH}/clang${PLATFORM_CLANG_VERSION}${TOOLCHAIN_SUFFIX}    CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER  ${PLATFORM_CLANG_PATH}/clang++${PLATFORM_CLANG_VERSION}${TOOLCHAIN_SUFFIX}  CACHE PATH "" FORCE)
set(CMAKE_SIZE          ${PLATFORM_CLANG_PATH}/llvm-size${TOOLCHAIN_SUFFIX}                         CACHE PATH "" FORCE)
set(CMAKE_AR            ${PLATFORM_CLANG_PATH}/llvm-ar${TOOLCHAIN_SUFFIX}                           CACHE PATH "" FORCE)
set(CMAKE_RANLIB        ${PLATFORM_CLANG_PATH}/llvm-ranlib${TOOLCHAIN_SUFFIX}                       CACHE PATH "" FORCE)
set(CMAKE_OBJCOPY       ${PLATFORM_CLANG_PATH}/llvm-objcopy${TOOLCHAIN_SUFFIX}                      CACHE PATH "" FORCE)

# dont try to create a shared lib, it will not work
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY                                                    CACHE INTERNAL "")

set(GD32_SPECS  "--specs=picolibc.specs" CACHE INTERNAL "" FORCE)
set(GD32_DEBUG_FLAGS "-g3 -gdwarf-2 -Oz" CACHE INTERNAL "")

# Architecture and MCU specific flags
set(GD32_MCU_C_FLAGS "--sysroot ${PLATFORM_CLANG_SYSROOT} ${PLATFORM_CLANG_C_FLAGS} -DLN_MCU=LN_MCU_CH32V3x -DLN_ARCH=LN_ARCH_RISCV" CACHE INTERNAL "")
set(GD32_C_FLAGS    "${GD32_SPECS} ${GD32_MCU_C_FLAGS} ${GD32_DEBUG_FLAGS} -Werror=return-type -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common" CACHE INTERNAL "")

set(CMAKE_C_FLAGS   "${GD32_C_FLAGS}" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "${GD32_C_FLAGS}" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS "${GD32_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics" CACHE INTERNAL "")

set(GD32_LD_FLAGS "-fuse-ld=lld -L${PLATFORM_CLANG_SYSROOT}/lib/riscv32-unknown-unknown-elf -nostdlib ${GD32_SPECS} --sysroot ${PLATFORM_CLANG_SYSROOT} -Wl,--warn-common" CACHE INTERNAL "")
set(GD32_LD_LIBS "-lm -Wl,--gc-sections -Wl,--gdb-index" CACHE INTERNAL "")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> ${PLATFORM_CLANG_C_FLAGS} <LINK_FLAGS> -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group -Wl,-Map,<TARGET>.map -o <TARGET> ${GD32_LD_FLAGS} ${GD32_LD_LIBS} ${GD32_LD_FLAGS} -lclang_rt.builtins" CACHE INTERNAL "")

set(CMAKE_EXECUTABLE_SUFFIX_C .elf CACHE INTERNAL "")
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf CACHE INTERNAL "")

# For stage1/stage2 linker options consistency
separate_arguments(CLANG_FLAGS NATIVE_COMMAND "${PLATFORM_CLANG_C_FLAGS}")
set(LD_FLAGS_COMMON
    ${CLANG_FLAGS}
    "-nostartfiles"
    "-Wl,--gc-sections"
    "-Wl,--warn-common"
)

macro(generate_firmware TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_options(${TARGET_NAME} PRIVATE -lc)
endmacro()
