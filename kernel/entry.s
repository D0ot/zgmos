.section .bss.stack
.align 16
.global startup_stack
.space 1024*16
startup_stack:

.section .text.start
.global _start
_start:
  la sp, startup_stack
  call main

# following is dummy code
_loop:
  li t0, 1
  j  _loop


  

