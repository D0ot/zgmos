.section .bss.stack
.align 16
.global startup_stack
.space 1024*16
startup_stack:
.global startup_stack2
.space 1024*16
startup_stack2:

.section .text.start
.global _start
_start:
  bne a0, x0, _start2
  la sp, startup_stack
  j _main

_start2:
  la sp, startup_stack2

_main:
  call main

# following is dummy code
_loop:
  li t0, 1
  j  _loop


  

