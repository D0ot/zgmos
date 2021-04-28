OUTPUT_ARCH("riscv")
ENTRY(_start)
PROVIDE(_kernel_entry = 0x80200000 );

MEMORY
{
  ram (wxa!ri) : ORIGIN = 0x80200000, LENGTH = (4*1024*1024)
  ram_nocahce (axa!ri) : ORIGIN = 0x40000000, LENGTH = (6*1024*1024)
}

PHDRS
{
  ram_ro    PT_LOAD;
  ram_init  PT_LOAD;
  ram       PT_NULL;
}


SECTIONS
{
  PROVIDE(_text_start = ABSOLUTE(.) );
  .text :
  {
    KEEP( *(.text.start) )
    *(.text .text.*)
  } >ram AT>ram :ram_ro
  PROVIDE(_text_end = ABSOLUTE(.) );

  . = ALIGN(4096);

  PROVIDE(_rodata_start = ABSOLUTE(.) );
  .rodata :
  {
    *.(.rdata)
    *.(.rodata .rodata.*)
  } >ram AT>ram :ram_ro
  PROVIDE(_rodata_end = ABSOLUTE(.) );

  . = ALIGN(4096);

  PROVIDE(_data_start = ABSOLUTE(.) );
  .data :
  {
    *(.data .data.*)
  } >ram AT>ram :ram_init
  PROVIDE(_data_end = ABSOLUTE(.) );

  . = ALIGN(4096);
  
  PROVIDE(_bss_start = ABSOLUTE(.) );
  .bss :
  {
    *(.bss .bss.*)
    *(.COMMON)
  } >ram AT>ram :ram
  PROVIDE(_bss_end = ABSOLUTE(.) );
}
