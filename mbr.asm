;code section
bits 16
org 0x7c00
;; just print something
mov bx,HELLO_STRING
call printl

mov bx,HELLO_STRING
call printl

;init seg registers
mov bp, 0x8000 ; set the stack safely away from us
mov sp, bp

mov bp, 0
mov es, bp

mov bp, 0
mov ds, bp

mov bp, 0
mov ss, bp



;load 512 byte to memery and print
mov bx, 0x9000 ; es:bx = 0x0000:0x9000 = 0x09000
mov dh, 2 ; read 2 sectors
call disk_load
mov bx, 0x9000+512; first word from second loaded sector, 0xface
call printl
jmp $



printl:
    call print
    call print_nl
    ret

;print string 
;string_base in ebx
print:
    mov ah, 0x0e ; tty mode
    .loop_start:
    mov al,[bx]
    int 0x10
    cmp  al,0
    je .loop_end
    add bx,1
    jmp .loop_start
    .loop_end:
    ;print new line
    ret


;print new line
print_nl:
    ;NL EQU  0x0a,0x0d
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    ret

%include "disk.asm"


;data section
HELLO_STRING:
db "hello world",0
times 510-($-$$) db 0
dw 0xAA55

MDFK_STRING:
db "MDFK",0
times 507 db 0

MDFK_STRING2:
db "MDFK2",0
times 506 db 0

