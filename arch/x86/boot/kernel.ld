
ENTRY(startup)

SECTIONS {
        . = 0x100000;
        
        .stext  : {
                __stext_start = .;
                *(.stext)
                __stext_end = .;
        }

        .text   : {
                __text_start = .;
                *(.text)
                __text_end = .;
        }

        .rodata : {
                __rodata_start = .;
                *(.rodata*)
                __rodata_end = .;
        }

        .data   : {
                __data_start = .;
                *(.data)
                __data_end = .;
        }

        .bss    : {
                __bss_start = .;
                *(.bss)
                __bss_end = .;
        }

        .debug  : {
                *(.debug*)
        }
        
}