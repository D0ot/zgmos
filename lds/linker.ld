OUTPUT_ARCH("riscv")
ENTRY(_start)
PROVIDE(_kernel_entry = 0x80200000 );

MEMORY
{
  sbi (wxa!ri) : ORIGIN = 0x80000000, LENGTH = (2*1024*1024)
  ram (wxa!ri) : ORIGIN = 0x80200000, LENGTH = (4*1024*1024)
  ram_nocahce (axa!ri) : ORIGIN = 0x40000000, LENGTH = (6*1024*1024)
}

PROVIDE(_ram_end = ORIGIN(ram) + LENGTH(ram) );

PROVIDE(_sbi_start = ORIGIN(sbi));
PROVIDE(_sbi_end = ORIGIN(sbi) + LENGTH(sbi));

PHDRS
{
  ram_text    PT_LOAD;
  ram_rodata  PT_LOAD;
  ram_data    PT_LOAD;
  ram_bss     PT_NULL;
}


SECTIONS
{
  .text :
  {
    PROVIDE(_kernel_start = ABSOLUTE(.) );
    PROVIDE(_text_start = ABSOLUTE(.) );
    KEEP( *(.text.start) )
    *(.text .text.*)
  } >ram AT>ram :ram_text

  PROVIDE(_text_end = ABSOLUTE(.) );
  PROVIDE(_rodata_start = ABSOLUTE(.) );
  .rodata :
  {
    *.(.rdata)
    *.(.rodata .rodata.*)
  } >ram AT>ram :ram_rodata

  . = ALIGN(4096);

  PROVIDE(_rodata_end = ABSOLUTE(.) );
  PROVIDE(_data_start = ABSOLUTE(.) );
  .data :
  {
    *(.data .data.*)
  } >ram AT>ram :ram_data
  . = ALIGN(4096);

  PROVIDE(_data_end = ABSOLUTE(.) );
  PROVIDE(_bss_start = ABSOLUTE(.) );
  .bss :
  {
    *(.bss .bss.*)
  } >ram AT>ram :ram_bss
  . = ALIGN(4096);
  
  PROVIDE(_bss_end = ABSOLUTE(.) );
  PROVIDE(_kernel_end = ABSOLUTE(.) );
}
