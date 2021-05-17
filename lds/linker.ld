OUTPUT_ARCH("riscv")
ENTRY(_start)
PROVIDE(_kernel_entry = 0x80200000 );

/*
MEMORY
{
  sbi (wxa!ri) : ORIGIN = 0x80000000, LENGTH = (2*1024*1024)
  ram (wxa!ri) : ORIGIN = 0x80200000, LENGTH = (4*1024*1024)
  ram_nocahce (axa!ri) : ORIGIN = 0x40000000, LENGTH = (6*1024*1024)
}*/

PROVIDE(_ram_end = 0x80800000);

PROVIDE(_sbi_start = 0x80000000);
PROVIDE(_sbi_end = 0x80200000);


SECTIONS
{
  . = _sbi_end;
  . = ALIGN(4K);
  .text :
  {
    PROVIDE(_kernel_start = ABSOLUTE(.) );
    PROVIDE(_text_start = ABSOLUTE(.) );
    KEEP( *(.text.start) )

    
    *(.text .text.*)
    . = ALIGN(4K);
    PROVIDE(_uvec_start = ABSOLUTE(.) );
    KEEP( *(.uvec) )
    . = ALIGN(4K);
    PROVIDE(_uvec_end = ABSOLUTE(.) );
  }

  . = ALIGN(4K);

  PROVIDE(_text_end = ABSOLUTE(.) );
  PROVIDE(_rodata_start = ABSOLUTE(.) );
  .rodata :
  {
    *.(.rdata)
    *.(.rodata .rodata.*)
  }

  /* . = ALIGN(8);*/
  . = ALIGN(4K);

  PROVIDE(_rodata_end = ABSOLUTE(.) );
  PROVIDE(_data_start = ABSOLUTE(.) );
  .data :
  {
    *(.data .data.*)
  }
  . = ALIGN(4K);

  PROVIDE(_data_end = ABSOLUTE(.) );
  PROVIDE(_bss_start = ABSOLUTE(.) );
  .bss :
  {
    *(.bss .bss.*)
  }
  . = ALIGN(4K);
  
  PROVIDE(_bss_end = ALIGN(4096) );
  PROVIDE(_kernel_end = ALIGN(4096) );
}
