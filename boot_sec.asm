[bits 16]
[org 0x7c00]
; Infinite loop (e9 fd ff)
;mov bx,string_hw
;mov dx,0x0800
;call print_str
;mov dx,0xc87A
;call print_hex

mov bp , 0x8000
mov sp , bp

call read_boot_sec

;check first byte read from disk (floppy)






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
times 512 dw 0x8763 ; sector 2 = 512 bytes
times 256 dw 0xface ; sector 3 = 512 bytes