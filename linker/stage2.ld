/* Linker script for Stage 2 (RAM-resident DFU handler) for RISC-V CH32V3x.
 * VMA = upper half of RAM (0x20008000), LMA = flash (embedded via binary).
 * Stage 1 copies the entire Stage 2 image to RAM, then jumps.
 * Stage 2 only needs to zero .bss (data is already at VMA after the copy).
 */

MEMORY
{
  /* upper 32 kB of the 64 kB RAM */
  code (rwx) :   ORIGIN = 0x20004000, LENGTH = 16K
  ram (rw) :   ORIGIN = 0x20008000, LENGTH = 32K
}

REGION_ALIAS("REGION_TEXT",   code);
REGION_ALIAS("REGION_RODATA", code);
REGION_ALIAS("REGION_DATA",   ram);
REGION_ALIAS("REGION_BSS",    ram);

OUTPUT_FORMAT("elf32-littleriscv")

ENTRY(stage2_entry)
__stack_size = DEFINED(__stack_size) ? __stack_size : 1024;

SECTIONS
{
    .text :
    {
        __text_start__ = .;
        KEEP(*(.text.entry))   /* stage2_entry (sets SP, does init) */
        *(.text .text.* .gnu.linkonce.t.*)
        . = ALIGN(4);
        KEEP(*(.init))
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;
        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array))
        __fini_array_end = .;
        . = ALIGN(8);
        __text_end__ = .;
    } > REGION_TEXT

    .rodata :
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
    } > REGION_RODATA


    PROVIDE(__data_lma__ = .);
    .data :
    {
        PROVIDE(__data_begin__ = .);
        . = ALIGN(8);
        *(.sdata*)
        *(.gnu.linkonce.s.*)
        *(.got.plt) *(.got)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE(__data_end__ = .);
    } > REGION_DATA AT> REGION_TEXT

    .bss :
    {
        . = ALIGN(8);
        PROVIDE(__bss_begin__ = .);
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(8);
        PROVIDE(__bss_end__ = .);
        _end = __bss_end__;
    } > REGION_BSS

    PROVIDE(_sp = ORIGIN(ram) + LENGTH(ram));

    /* Debug sections */
    .stab        0 (NOLOAD) : { *(.stab) }
    .stabstr     0 (NOLOAD) : { *(.stabstr) }
    .debug       0 : { *(.debug) }
    .line        0 : { *(.line) }
    .debug_info  0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line  0 : { *(.debug_line) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_str   0 : { *(.debug_str) }
    /DISCARD/ : { *(.note.GNU-stack) }
}
