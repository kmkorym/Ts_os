[bits 16]
[org 0x7c00]
BOOT_CODE_SEGMENT equ 0x1000 
BOOT_CODE_OFFSET equ 0x0000 
BOOT_CODE_SEGNUM equ 2
;[es:bx] cant exceed 0x9fc00


mov bp , 0x8000
mov sp , bp

call read_boot_sec

;check first byte read from disk (floppy)



mov ax,BOOT_CODE_SEGMENT 
mov es,ax
mov bx,BOOT_CODE_OFFSET
mov dx,[es:bx+512]
call print_hex

loop:
    jmp loop 

%include "./print.asm"
%include "./read_disk.asm"
string_hw:
    db  'Hello, World', 0

; Fill with 510 zeros minus the size of the previous code
times 510-($-$$) db 0
; Magic number
dw 0xaa55 
times 256 dw 0x8763 ; sector 2 = 512 bytes
times 256 dw 0x1234 ; sector 3 = 512 bytes