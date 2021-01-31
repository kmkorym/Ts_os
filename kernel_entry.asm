section .text
global _start
_start:
mov ebx,TD
[bits 32]
[extern fake_print]
# call  fake_print
jmp $

TD:
    dw "test data",0
