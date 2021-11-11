section .text
[bits 32]
[extern main]
[extern exit]
start:
call main
call exit


