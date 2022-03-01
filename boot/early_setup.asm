[bits 32]
; this file is for early set up procedure before kernel entry
; process before kernel etry point is executed needs to do in this pbase
; for example setup page table for higher half kernel
[extern  pg_dir0_phy ]
[extern  init_pg_dir0_vmap]

global _start_early_init:
_start_early_init:

cli
call  init_pg_dir0_vmap

; enable page
 mov ebx, pg_dir0_phy
 mov  eax,[ebx]
 mov cr3, eax
 mov eax, cr0
 or eax, 0x80000001
 mov cr0, eax

 [extern _init_end]
 ;mov eax,_init_end
 ;mov [eax],eax
 mov ebx,_init_end
 jmp ebx


 

; the code after this line use link address of higher kernel
; use cat command to generate image to ensure code order is correct

