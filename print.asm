hex_string_template:
    db '0x0000',0


;dx : number
print_hex:
    pusha
    mov cx,4
    mov bx,hex_string_template
    add bx,5
    print_hex_loop:
        cmp cx,0
        je  print_hex_end
        mov al,dl
        and al,00001111b
        cmp al,10
        jl aaa
        add al,7
        aaa:
        add al,48
        mov byte [bx],al
        shr dx,4
        sub bx,1
        sub cx,1
        jmp print_hex_loop

    print_hex_end:
        mov bx,hex_string_template
        mov dx,0x0800
        call print_str
        popa
        ret       


;bx : address of string_hw
;dh: row ,dl : col
print_str:
    pusha
    mov ax,0xb800
    mov es,ax
    ;mov h,0x0f ; white on black (character color)
    ;mov al,'X'
    ;https://stackoverflow.com/questions/1797765/assembly-invalid-effective-address
    ;Invalid effective address

    ;calculate adress by row and col
    mov ax,0
    ;https://www.csie.ntu.edu.tw/~acpang/course/asm_2004/slides/chapt_07_PartIISolve.pdf
    mov al,dh
    mov cl,0x50
    mul cl
    mov dh,0
    add ax,dx
    shl ax,1
    ;write data to vga
    ;Each start of iteration
    ;bx base address of string_hw
    ;ax base  address of vga
    print_loop:
        mov dl,[bx]
        cmp dl,0
        je print_end
        mov cx,bx
        mov bx,ax
        ;mov ah,0x0f
        mov byte[es:bx],dl
        mov ax,bx
        mov bx,cx

        add bx,1
        add ax,0x0002
        jmp print_loop

    print_end:
        popa
        ret


print_nl:
    pusha
    mov ah, 0x0e
    mov al, 0xa
    int 0x10
    mov al, 0xd
    int 0x10
    popa
    ret