global _start
section .text
_start:
[bits 32]
[extern main]
call  main
[extern test_call]
pusha
push 0x55
push 0x66

call test_call
popa
# sti
# sti
jmp $

