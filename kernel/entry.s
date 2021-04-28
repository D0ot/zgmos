.section .bss.stack
.align 16
.global startup_stack
startup_stack:
.space 1024*16

.section .text.start
.global _start
_start:
  la sp, startup_stack
  call main
_loop:
  j  _loop


  

