;code section
bits 16
org 0x7c00

mov ebx,HELLO_STRING
call printl

mov ebx,HELLO_STRING
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
    mov al,[ebx]
    int 0x10
    cmp  al,0
    je .loop_end
    add ebx,1
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



;data section
HELLO_STRING:
db "hello world",0
times 510-($-$$) db 0
dw 0xAA55


