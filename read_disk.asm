read_boot_sec:
    pusha
    mov ax,0xB000
    mov es,ax
    mov bx,0x0000
    mov ch,0
    mov cl,2
    mov dh,0

    mov ah,2
    mov al,2
    int 13
 
    jc disk_err
    cmp al,2 ; must read 2 sectors
    je disk_read_complete
    ; read error
    mov bx,str_disk_err2
    mov dx,0x0900
    call print_str
    mov dh,0
    mov dl,al
    call print_hex

    jmp fuck


    disk_read_complete:
        ;mov ah, 0x0e
        ;mov al,[es:bx]
        ;int 0x10
        mov ax,0xB000
        mov bx,0x0000
        mov dx,[es:bx]
        call print_hex
        popa
        ret


    disk_err:
        mov bx,str_disk_err
        mov dx,0x0800
        call print_str
        jmp fuck

    fuck:
        mov dx,0x8787
        call print_hex
        jmp fuck


str_disk_err:
    db 'error while read disk',0
str_disk_err2:
    db 'Incorrect number of sectors read',0