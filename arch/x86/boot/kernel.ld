
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

	/* DWARF sections (support up to 4) */
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_info	0 : { *(.debug_info) }
	.debug_line	0 : { *(.debug_line) }
	.debug_loc	0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
	.debug_pubnames	0 : { *(.debug_pubnames) }
	.debug_pubtypes	0 : { *(.debug_pubtypes) }
	.debug_ranges	0 : { *(.debug_ranges) }
	.debug_str	0 : { *(.debug_str) }
	.debug_types	0 : { *(.debug_types) }

	/* DWARFのversionが4の場合、その他の.debug*というELFセクションはないはず */
	.debug_others   0 : { *(.debug*) }
	ASSERT(SIZEOF(.debug_others) == 0, "Unexpected .debug* section detected!")

}
