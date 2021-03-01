section .text
[bits 32]
[extern main]
push ebp
mov ebp,esp
call main
leave
ret