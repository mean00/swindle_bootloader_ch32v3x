OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
    /* Stage 1 is the bootloader — it STARTS at flash address 0x0 */
    rom (rx)  : ORIGIN = 0x00000000, LENGTH = 32K
    ram (rwx) : ORIGIN = 0x20000010, LENGTH = 16K - 16
}

__stack_size = DEFINED(__stack_size) ? __stack_size : 1024;

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.init))
        . = ALIGN(4);
        KEEP(*(SORT_NONE(.init)))
        . = ALIGN(4);
        *(.text .text.* .gnu.linkonce.t.*)
        . = ALIGN(4);
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } >rom AT>rom

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } >rom AT>rom

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
        PROVIDE_HIDDEN(__init_array_end = .);
    } >rom AT>rom

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP(*(.fini_array EXCLUDE_FILE(*crtend.o *crtend?.o) .dtors))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } >rom AT>rom

    .ctors :
    {
        KEEP(*crtbegin.o(.ctors))
        KEEP(*crtbegin?.o(.ctors))
        KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
    } >rom AT>rom

    .dtors :
    {
        KEEP(*crtbegin.o(.dtors))
        KEEP(*crtbegin?.o(.dtors))
        KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
    } >rom AT>rom

    .data : ALIGN(8)
    {
        *(.gnu.linkonce.r.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.* .sdata2.*)
        *(.gnu.linkonce.s.*)
        . = ALIGN(8);
        *(.srodata.cst16)
        *(.srodata.cst8)
        *(.srodata.cst4)
        *(.srodata.cst2)
        *(.srodata .srodata.*)
        . = ALIGN(4);
    } >ram AT>rom

    PROVIDE(_data_begin = ADDR(.data));
    PROVIDE(_data_end = ADDR(.data) + SIZEOF(.data));
    PROVIDE(_data_lma = LOADADDR(.data));

    .bss (NOLOAD) : ALIGN(4)
    {
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
    } >ram

    PROVIDE(_bss_begin = ADDR(.bss));
    PROVIDE(_bss_end = ADDR(.bss) + SIZEOF(.bss));

    PROVIDE(_end = .);
    PROVIDE(end = .);

    PROVIDE(_sp = ORIGIN(ram) + LENGTH(ram));
}
