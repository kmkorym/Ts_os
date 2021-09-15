get_floppy_chs:
    mov dh,0
    mov ah, 0x08
    int 0x13
    jc get_floppy_error
    inc dh
    mov [HEAD_NUM],dh
    mov ax,cx
    and al,0x3f
    mov [SECTOR_NUM],al
    mov al,cl
    shr al,6 
    mov cl,ch
    mov ch,al
    inc cx
    mov [CYL_NUM],cx
    ret 

get_floppy_error:
    mov bx, GET_FLOPPY_ERROR
    call __printl
    jmp $


; bx, start address
; cl,ch, #of cylinder start/end index to read
; load  0 to cx-1  cylinder to base bx
disk_load_all:
    push dx
    push ax
    mov dx,0
    mov dl,cl
    .disk_read_loop:
        cmp dl,ch
        ; jg is singed comparision, use ja
        ja .disk_read_loop_end
        mov dh,0
        call load_cylinder_cross_64K
        mov dh,1
        call load_cylinder_cross_64K
        inc dl
        jmp .disk_read_loop
    .disk_read_loop_end:

    pop ax
    pop dx
    ret




; es:bx
; cx  --> number 
;update_disk_load_offset:


; es:bx : base address of memory
; dl : cylinder idx
; dh : head idx
; return es,bx may be modified (because change segemnt for boundary)
load_cylinder_cross_64K:
    push cx
    push dx

    mov ah,1                ; start sector
    mov al,[SECTOR_NUM] 
    cmp dl,0
    jne .not_cyl0
    cmp dh,0
    jne .not_cyl0
    inc ah
    dec al
    .not_cyl0:

    push ax 

    ;calcuate late if final address is cross boundary
   
    mov cx,0
    mov cl,al
    mov ax,512
    push dx
    mul cx          ; mul will modify dx with higher byte result , so save it
    pop dx
    mov cx,bx
    dec cx          ; case  0xFFFF
    add cx,ax
    jc .cross_64k
    pop ax
    call disk_load
    jmp .end_disk_req
    .cross_64k:
    ; first part byte count
    mov cx,0xFFFF
    sub cx,bx
    inc cx
    mov ax,cx
    mov cx,512
    push dx
    mov dx,0
    div cx
    pop dx
    cmp ah,0
    jne block_div_err
    
    
    mov cx,ax     ; first part sector -> cl
    pop ax        ; restore ah
    push ax
    mov al,cl
    call disk_load
    ; second part byte count
    pop ax
    sub al,cl
    add ah,cl
    mov cx,es
    add cx,0x1000
    mov es,cx
    mov bx,0
    call disk_load
    .end_disk_req:

    pop dx
    pop cx
    ret


;https://stanislavs.org/helppc/int_13-2.html
; assume max 255 cylinders
; es:bx : base address of memory
; al : # of sector to read 
; ah : start sector
; dl : cylinder idx
; dh : head idx
; return 
; new es:bx
disk_load:
    ; reading from disk requires setting specific values in all registers
    ; so we will overwrite our input parameters from 'dx'. Let's save it
    ; to the stack for later use.
    push ax
    push cx
    push dx
    ;xor ah, ah    ; set ah to zero - reset drive function
    ;int 0x13       ; call drive reset
    mov cl, ah            ; cl <- start sector (0x01 .. 0x11)
    mov ah, 0x02          ; ah <- int 0x13 function. 0x02 = 'read'
   ; mov al,0x39          ; al <- number of sectors to read (0x01 .. 0x80)
    mov ch, dl
    mov dl,0

    push ax
                            ;mov dh,dh 
                            ;BIOS int 13 AH 02
                            ;AL = number of sectors to read	(1-128 dec.)
                            ;CH = track/cylinder number  (0-1023 dec., see below)
                            ;CL = sector number  (1-17 dec.)
                            ;DH = head number  (0-15 dec.)
                            ;DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)


                            ; [es:bx] <- pointer to buffer where the data will be stored
                            ; caller sets it up for us, and it is actually the standard location for int 13h
    int 0x13      
    jc disk_error           ; if error (stored in the carry bit)
    ;                        ; BIOS also sets 'al' to the # of sectors read. Compare it.
    
    pop dx
    cmp al,dl
    jne sectors_error

    
    mov cx,0
    mov cl,al
    mov ax,512
    push dx
    mul cx
    pop dx


    ; check overflow boundary
    add bx,ax
    jc .carry_not_safe
    jmp .carry_safe
    .carry_not_safe:
    cmp bx,0
    je  .carry_safe
    jmp overflow_64_error
    .carry_safe:

    pop dx
    pop cx
    pop ax
    ret



disk_error:
    mov bx, DISK_ERROR
    call __printl
    ;mov dh, ah ; ah = error code, dl = disk drive that dropped the error
    ;call __print_hex ; check out the code at http://stanislavs.org/helppc/int_13-1.html
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call __printl
    jmp $

overflow_64_error:
    mov bx, OVERFLOW_64K_ERROR
    call __printl
    jmp $

block_div_err:
    mov bx, DIV_BLK_ERROR
    call __printl
    jmp $

disk_loop:
    jmp $

DIV_BLK_ERROR: db "block div r!=0", 0
OVERFLOW_64K_ERROR: db "64K Overflow", 0
DISK_ERROR: db "Disk read error", 0
SECTORS_ERROR: db "Incorrect number of sectors read", 0
GET_FLOPPY_ERROR: db "FLP Error", 0

CYL_NUM: dw 0       ; 0x50
HEAD_NUM: db 0      ; 2
SECTOR_NUM: db 0    ; 0x12  or  0x24


; 9000 or  9000/2  --> 1 cylinder
