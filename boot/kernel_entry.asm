global _start
section .text
_start:
[bits 32]
[extern main]
[extern test_call]
[extern  init_page_settings]
[extern irq_handler_entry]
[extern user_loop]

call  main

;tss_flush:
;mov ax, 0x2B     
;ltr ax  
mov eax,IDT_TABLE_DESC
lidt [eax]


; must init user space page tables ...
;call init_page_settings

;[extern switch_to_user]
;[extern user_loop]
;push USER_START
;call switch_to_user

;USER_START:
;_ULOOP:
;call user_loop

sti
call user_loop



global IDT_TABLE_DESC
IDT_TABLE_DESC:
    dw 0
    dd 0

%macro ISR_NOERRCODE 1  ; define a macro, taking one parameter
  [GLOBAL isr%1]        ; %1 accesses the first parameter.
  isr%1:
    cli
    pusha
    push dword 0
    push dword %1
    call irq_handler_entry
    add esp, 8
    popa
    sti
    iret   
%endmacro

%macro ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push eax
    mov  eax,[esp+4]
    pusha
    push eax      ; error code
    push dword %1 ; isr number  
    call irq_handler_entry
    add esp,8
    popa
    pop eax
    add esp,4
    sti
    iret
%endmacro


ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
GLOBAL isr13
[extern   kill_and_reschedule  ]
isr13:
  cli
  add esp,8 ; error code + original eip
  push dword  kill_and_reschedule
  sti
iret
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47
ISR_NOERRCODE 48
ISR_NOERRCODE 49
ISR_NOERRCODE 50
ISR_NOERRCODE 51
ISR_NOERRCODE 52
ISR_NOERRCODE 53
ISR_NOERRCODE 54
ISR_NOERRCODE 55
ISR_NOERRCODE 56
ISR_NOERRCODE 57
ISR_NOERRCODE 58
ISR_NOERRCODE 59
ISR_NOERRCODE 60
ISR_NOERRCODE 61
ISR_NOERRCODE 62
ISR_NOERRCODE 63



[SECTION .kernel_setup]
[extern _kernel_setup_end]
[extern __kernel_size]
mov eax,_kernel_setup_end
add ebx,eax
mov ecx, __kernel_size
mov edx,0xC0800000


; start mov kernel
copy_kernel:
cmp ecx,0
je end_copy_kernel
mov  eax,[ebx]
mov  [edx],eax
inc edx
inc ebx
dec ecx
jmp copy_kernel ; //this is relative jump
end_copy_kernel:
mov edx,0xC0800000
jmp edx
