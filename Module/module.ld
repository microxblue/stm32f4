/* Entry Point */
ENTRY(module_start)

/* Memories definition */
MEMORY
{
  RAM     (xrw)    : ORIGIN = @FLASHBASE,    LENGTH = 0x20010000 - @FLASHBASE - 0x400
}

/* Section Definitions */
SECTIONS
{
    .text :
    {
        *(.entry)
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
    } >RAM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >RAM

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >RAM

    . = ALIGN(4);
    _etext = .;
    __etext = .;
    _sidata = .;

    .data : AT (_etext)
    {
        __data_start__ = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end__ = .;
    } >RAM

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        __bss_start__ = . ;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
         __bss_end__ = . ;
    } >RAM

    _end = . ;
}
