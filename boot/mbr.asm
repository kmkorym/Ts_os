; bootloader 1
; because real mode max address is 1MB and real mode memory map > 0x80000 is for system usage
; so kernel size can't bigger than 0.5 MB if want to use BIOS to load kernel in real mode
; TODO to fix it to write a phase 2 bootloader in protected mode and use floppy/hdd driver
; to read full kernel ...

;code section
[bits 16]
; 
LOADED_ADDRESS equ 0x9000
LOADED_CYL1_NUM equ  25                   ;(0x80000-0x9000)/(0x4800)
LOADED_CYL2_NUM equ  12                   ;(0x80000-0x9000)/(0x9000)
[org 0x7c00]
;; just print something
;mov bx,HELLO_STRING
;call __printl

;mov bx,HELLO_STRING
;call __printl

;init seg registers
mov bp, 0x8000 ; set the stack safely away from us
mov sp, bp

mov bp, 0
mov es, bp

mov bp, 0
mov ds, bp

mov bp, 0
mov ss, bp

call get_floppy_chs

;after call get flopy, es ,di wil be modified so reset
mov bp, 0
mov es, bp
mov di,0

;load 1024 byte to memery and print
mov bx, LOADED_ADDRESS ; es:bx = 0x0000:0x9000 = 0x09000
mov cl,0

; because memory  0x00080000 ~ 0x000FFFFFF is for BIOS , VGA .. 
; and other system mapping so can't load kernel in that region
mov ch,LOADED_CYL1_NUM
mov al,[SECTOR_NUM]
cmp al,0x12 
je MB1_SEC
mov ch,LOADED_CYL2_NUM
MB1_SEC:
call disk_load_all
mov bx,HELLO_STRING
call __printl
;mov bx, 0x9000+512; first word from second loaded sector, 0xface
; ----------------------------------------------------
;mov dx,0
;call disk_load


;mov bx,HELLO_STRING
;call __printl


;switch to protected mode
cli;
mov eax,GDT_TABLE_DESC
lgdt [eax]

mov eax , cr0 ; To make the switch to protected mode , we set
or eax , 0x1 ; the first bit of CR0 , a control register
mov cr0 , eax
mov ebp, 0x90000 ; 0x9000 ~ 0x9000+1MB is set page identity mapping , must let ebp esp with safe range
mov esp, 0x90000
mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
mov ds, ax        ; Load all data segment selectors
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
jmp 0x08:(START32-GDT_BASE+LOADED_ADDRESS)

__printl:
    call __print
    call __print_nl
    ret

;print string 
;string_base in ebx
__print:
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
__print_nl:
    ;NL EQU  0x0a,0x0d
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    ret



%include "boot/disk.asm"


;data section
HELLO_STRING:
db "hello world",0
;MDFK_STRING2:
;db "MDFK2",0

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
USR_CODE:
dw 0xFFFF ; limit
dw 0x00 ;base
db 0x00 ; base
db 11111010b     ; access byte
db 11001111b     ; flag G=1 D=1
db 0x00 ; base
USR_DATA:
dw 0xFFFF ; limit
dw 0x00 ; base
db 0x00 ; base
db 11110010b    ; access byte
db 11001111b     ; flag G=1 D=1
db 0x00 ; base
TSS_SEGMENT:
dw 0x00 ; limit
dw 0x00 ; base
db 0x00 ; base
db 0x00   ; access byte
db 0x00     ; flag G=1 D=1
db 0x00 ; base
GDT_END:
START32: