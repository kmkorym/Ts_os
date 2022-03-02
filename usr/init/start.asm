section .text
[bits 32]
[extern main]
[extern exit]
start:
mov eax,ebx
mov eax,ecx
mov ecx,eax
call main
call exit


