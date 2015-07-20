/*
 * linker script for STM32F107 (256kB Flash, 64kB SRAM ) with GNU ld
 * bernard.xiong 2009-10-14
 *
 * stm32f103vf 768kB flash, 96KB sram, 最后2KB用于存储系统数据
 *
 * 前30KiB用于存储boot
 */


/*
 * ALIGN 返回当前的定位计数器，并向上对齐到指定的值
 */
/* Program Entry, set to mark it as "used" and avoid gc */
/*
 * 0x3000 -- 12K
 * 0x4000 -- 16K
 * 0x7800 -- 30K
 */
MEMORY
{
	/* CODE (rx) : ORIGIN = 0x08000000+0x7800, LENGTH = (768 - 30 - 2)K */
	CODE (rx)    	: org = 0x08007800, len = 736K
	DATA (rw)    	: org = 0x20000000, len = 96K
}

FLASH_START_ADD		= 0x08000000+0x7800;
FLASH_LEN		= 768K - 30K - 2K;
I_SRAM_START_ADD	= 0x20000000;
I_SRAM_LEN		= 64K;

ENTRY(Reset_Handler)


/* 0x5c6c */
_system_stack_size = 0x400;
_estack = I_SRAM_START_ADD + I_SRAM_LEN;

SECTIONS
{
	.text :
	{
		. = ALIGN(4);
		_isr_vector_start = ABSOLUTE(.);
		KEEP(*(.isr_vector))            /* Startup code */
		. = ALIGN(4);
		*(.text)                        /* remaining code */
		*(.text.*)                      /* remaining code */
		*(.rodata)                      /* read-only data (constants) */
		*(.rodata*)
		*(.glue_7)
		*(.glue_7t)
		*(.gnu.linkonce.t*)

		/* section information for finsh shell */
		. = ALIGN(4);
		__fsymtab_start = .;
		KEEP(*(FSymTab))
		__fsymtab_end = .;
		. = ALIGN(4);
		__vsymtab_start = .;
		KEEP(*(VSymTab))
		__vsymtab_end = .;
		. = ALIGN(4);

		. = ALIGN(4);
		_etext = .;
	} >CODE =0

	/* .ARM.exidx is sorted, so has to go in its own output section.  */
	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)

		/* This is used by the startup in order to initialize the .data secion */
		_sidata = .;
	} >CODE
	__exidx_end = .;

	/*
	 * DATA  ---- add by David
	 * .data
	 * .bss
	 * .heap
	 * .stack
	 */
	/* .data section which is used for initialized data */
	.data : AT (_sidata)
	{
		. = ALIGN(4);
		/* This is used by the startup in order to initialize the .data secion */
		_sdata = . ;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)

		. = ALIGN(4);
		/* This is used by the startup in order to initialize the .data secion */
		_edata = . ;
	} >DATA

	__bss_start = .;
	.bss :
	{
		. = ALIGN(4);
		/* This is used by the startup in order to initialize the .bss secion */
		_sbss = .;

		*(.bss)
		*(.bss.*)
		*(COMMON)

		. = ALIGN(4);
		/* This is used by the startup in order to initialize the .bss secion */
		_ebss = . ;        
		*(.bss.init)
	} >DATA
	__bss_end = .;

	/* _end = .; */

	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	/* DWARF debug sections.
	* Symbols in the DWARF debugging sections are relative to the beginning
	* of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}
