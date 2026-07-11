#=============================================================================#
# Two-stage bootloader for CH32V3x (RISC-V)
# Stage 1: Flash resident — DFU decision, copy stage2, chain
# Stage 2: RAM resident — DFU handler via polled USB OTG FS
#=============================================================================#
cmake_minimum_required(VERSION 3.13)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "==========================================")
message(STATUS "swindle_bootloader_ch32v3x — two-stage build")
message(STATUS "==========================================")

# ---- Configuration ----
include(${CMAKE_SOURCE_DIR}/bootloader_config.cmake)
include(${CMAKE_SOURCE_DIR}/mcuSelect.cmake)
include(${CMAKE_SOURCE_DIR}/platformConfig.cmake)

# ---- Toolchain setup ----
option(USE_CLANG "Use Clang instead of GCC" OFF)

set(USE_CLANG True)

if(USE_CLANG)
  include(${CMAKE_SOURCE_DIR}/toolchain_clang.cmake)
else()
  include(${CMAKE_SOURCE_DIR}/toolchain_gcc.cmake)
endif()

# ---- Project ----
project(swindle_bootloader_ch32v3x NONE)
enable_language(C CXX ASM)

# ---- Source directories ----
set(ESPRIT_MINI_INC   ${CMAKE_SOURCE_DIR}/esprit_mini/include)
set(RISCV_CH32_INC    ${CMAKE_SOURCE_DIR}/riscv_ch32v3x)
set(SRC_COMMON        ${CMAKE_SOURCE_DIR}/src_common)
set(SRC_STAGE1        ${CMAKE_SOURCE_DIR}/src_stage1)
set(SRC_STAGE2        ${CMAKE_SOURCE_DIR}/src_stage2)

# Common include directories
include_directories(${ESPRIT_MINI_INC} ${RISCV_CH32_INC} ${SRC_COMMON} ${CMAKE_BINARY_DIR})

# Override LN_BOOTLOADER_SIZE for the linker script (set to 0 by mcuSelect.cmake)
set(LN_BOOTLOADER_SIZE ${BOOTLOADER_SIZE_KB} CACHE INTERNAL "")

# Generate memory_config.h from template
set(SEPSEP "/")
configure_file("${SRC_COMMON}/memory_config.h.in" "${CMAKE_BINARY_DIR}/memory_config.h" @ONLY)

# Generate flash_config.h (used by both stages)
configure_file("${CMAKE_SOURCE_DIR}/flash_config.h.in" "${CMAKE_BINARY_DIR}/flash_config.h" @ONLY)

# ---- Esprit-mini common sources (used by both stages) ----
set(ESPRIT_MINI_SRCS
    ${CMAKE_SOURCE_DIR}/esprit_mini/gpio.cpp
    ${CMAKE_SOURCE_DIR}/esprit_mini/system_timer.cpp
    ${CMAKE_SOURCE_DIR}/esprit_mini/debug.cpp
    ${CMAKE_SOURCE_DIR}/esprit_mini/peripheral.cpp
    ${CMAKE_SOURCE_DIR}/esprit_mini/rcu.cpp
)

# Common sources (used by both stages)
set(COMMON_SRCS
    ${SRC_COMMON}/stubs.cpp
)

# ---- Stage 2 (RAM resident) ----
set(STAGE2_SRCS
    ${SRC_STAGE2}/dfu.cpp
    ${SRC_STAGE2}/stage2_entry.cpp
    ${SRC_STAGE2}/stage2_irq.cpp
    ${SRC_STAGE2}/usb.cpp
    ${ESPRIT_MINI_SRCS}
    ${CMAKE_SOURCE_DIR}/esprit_mini/irq.cpp
    ${COMMON_SRCS}
)

generate_firmware(stage2.elf ${STAGE2_SRCS})

# Custom linker script for stage2 (RAM)
target_link_options(stage2.elf PRIVATE
    "-T${CMAKE_SOURCE_DIR}/linker/stage2.ld"
    "-Wl,-Map,${CMAKE_BINARY_DIR}/stage2.map"
    ${LD_FLAGS_COMMON}
)

# Post-process: stage2.elf → stage2.bin → stage2_payload.h
set(STAGE2_BIN  ${CMAKE_BINARY_DIR}/stage2.bin)
set(STAGE2_H    ${CMAKE_BINARY_DIR}/stage2_payload.h)

add_custom_command(TARGET stage2.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:stage2.elf> ${STAGE2_BIN}
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/bin2h.py ${STAGE2_BIN} > ${STAGE2_H}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating stage2 binary and header"
)

# ---- Stage 1 (Flash resident) ----
# Stage1 needs the generated header, so add a dependency
set(STAGE1_SRCS
    ${SRC_STAGE1}/main.cpp
    ${SRC_STAGE1}/ch32_crc.c
    ${ESPRIT_MINI_SRCS}
    ${COMMON_SRCS}
    ${RISCV_CH32_INC}/start.cpp
    ${RISCV_CH32_INC}/start.S
)

generate_firmware(stage1.elf ${STAGE1_SRCS})
add_dependencies(stage1.elf stage2.elf)

# Custom linker script: stage1 starts at flash address 0x00000000 (we ARE the bootloader)
set(STAGE1_LD "${CMAKE_SOURCE_DIR}/linker/stage1.ld")

target_include_directories(stage1.elf PRIVATE ${CMAKE_BINARY_DIR})
target_link_options(stage1.elf PRIVATE
    "-T${STAGE1_LD}"
    "-Wl,-Map,${CMAKE_BINARY_DIR}/stage1.map"
    ${LD_FLAGS_COMMON}
)

# Fix startup — use our start.S, not the default one
target_link_options(stage1.elf PRIVATE
    -Wl,-e,_start
)

# Post-process stage1
add_custom_command(TARGET stage1.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:stage1.elf> $<TARGET_FILE:stage1.elf>.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:stage1.elf>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating stage1 binary and size summary"
)

# ---- Summary ----
message(STATUS "============= Build Summary =============")
message(STATUS "MCU:        CH32V3x (RISC-V ${MARCH}/${MABI})")
message(STATUS "Toolchain:  ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}")
message(STATUS "Bootloader: ${BOOTLOADER_SIZE_KB} KB")
message(STATUS "App:        ${APP_SIZE_KB} KB")
message(STATUS "==========================================")
message(STATUS "Stage1 src: ${STAGE1_SRCS}")
message(STATUS "Stage2 src: ${STAGE2_SRCS}")
message(STATUS "==========================================")
