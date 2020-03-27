[bits 16]
[org 0x7c00]
BOOT_CODE_SEGMENT equ 0x5000 
BOOT_CODE_OFFSET equ 0x0000 
BOOT_CODE_SEGNUM equ 2
GDT_CODE_SEG equ gdt_code-gdt_start
GDT_DATA_SEG equ gdt_data-gdt_start
GDT_BASE equ gdt_start 
GDT_SIZE equ gdt_end - gdt_start 
;[es:bx] cant exceed 0x9fc00


mov bp , 0x8000
mov sp , bp

;call read_boot_sec

;check first byte read from disk (floppy)



;mov bx,BOOT_CODE_SEGMENT 
;mov es,bx
;mov bx,BOOT_CODE_OFFSET
;mov dx,[es:bx+24]
;call print_hex

;prepare gdt descriptor


cli
lgdt [gdt_desc] 
mov eax , cr0 ; To make the switch to protected mode , we set
or eax , 0x1 ; the first bit of CR0 , a control register
mov cr0 , eax
jmp 08h:protected_mode_start

string_hw:
    db  'Big Hamemer AAAAAAAAA', 0
;define gdt
gdt_start:
    dq 0
gdt_code:
    dw 0xFFFF ; limit
    dw 0x0000 ; base
    db 0; base
    db 10011010b ;一個一個定義byte endian問題 . 
    db 11001111b
    db 0
gdt_data:
    dw 0xFFFF ; limit
    dw 0x0000 ; base
    db 0; base
    db 10010010b
    db 11001111b
    db 0
gdt_end:
gdt_desc:
    dw GDT_SIZE-1
    dd gdt_start


%include "./print.asm"
%include "./read_disk.asm"
protected_mode_start:
[bits 32]
mov ax, 16
mov ds, ax
mov ss, ax
mov esp,0x8000
; print hello World
mov ebx,string_hw
call print_string_pm
; Fill with 510 zeros minus the size of the previous code
loop:
    jmp loop
times 510-($-$$) db 0
; Magic number
dw 0xaa55 
times 232 dw 0x8763 ; sector 2 = 512 bytes
times 256 dw 0x1234 ; sector 3 = 512 bytes