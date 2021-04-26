.section .text.start
.global _start
_start:
  jal main
_loop:
  j  _loop
  

