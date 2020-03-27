
[bits 16]
read_boot_sec:
    pusha
    mov ax,BOOT_CODE_SEGMENT 
    mov es,ax
    mov bx,BOOT_CODE_OFFSET
    mov ch,0
    mov cl,2
    mov dh,0

    mov ah,2
    mov al,BOOT_CODE_SEGNUM
    int 0x13 ; 13 --> 0x13 ...
 
    jc disk_err
    cmp al,BOOT_CODE_SEGNUM ; must read 2 sectors
    je disk_read_complete
    ; read error
    mov bx,str_disk_err2
    mov dx,0x0900
    call print_str
    mov dh,0
    mov dl,al
    call print_hex

    jmp halt_for_error

    disk_read_complete:
        popa
        ret
    
    disk_err:
        mov bx,str_disk_err
        mov dx,0x0800
        call print_str
        jmp halt_for_error

    halt_for_error:
        jmp halt_for_error



str_disk_err:
    db 'error while read disk',0
str_disk_err2:
    db 'Incorrect number of sectors read',0