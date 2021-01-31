;code section
[bits 16]
LOADED_ADDRESS equ 0x9000
KERNEL_ENTRY_ADDRESS equ 0x9018
[org 0x7c00]
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



;load 1024 byte to memery and print
mov bx, LOADED_ADDRESS ; es:bx = 0x0000:0x9000 = 0x09000
mov dh, 2 ; read 2 sectors
call disk_load
;mov bx, 0x9000+512; first word from second loaded sector, 0xface
;call printl
;jmp $

;switch to protected mode

cli;
mov eax,GDT_TABLE_DESC
lgdt [eax]

mov eax , cr0 ; To make the switch to protected mode , we set
or eax , 0x1 ; the first bit of CR0 , a control register
mov cr0 , eax

mov esp, 0x90000
mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
mov ds, ax        ; Load all data segment selectors
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
jmp 0x08:(START32-GDT_BASE+LOADED_ADDRESS)

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
MDFK_STRING2:
db "MDFK2",0

GDT_TABLE_DESC:
    dw GDT_END-GDT_BASE-1 ; size of table
    dd LOADED_ADDRESS

times 510-($-$$) db 0
dw 0xAA55
GDT_BASE:
dw 0x00 ; NULL selector
dw 0x00
dw 0x00
dw 0x00
GDT_CODE:
dw 0xFFFF ; limit
dw 0x00 ;base
db 0x00 ; base
db 10011010b     ; access byte
db 11001111b     ; flag G=1 D=1
db 0x00 ; base
GDT_DATA:
dw 0xFFFF ; limit
dw 0x00 ; base
db 0x00 ; base
db 10010010b    ; access byte
db 11001111b     ; flag G=1 D=1
db 0x00 ; base
GDT_END:
# here is 0x9018
START32:

# [bits 32]
# call  KERNEL_ENTRY_ADDRESS


