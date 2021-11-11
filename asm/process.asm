[GLOBAL read_eip]
[GLOBAL save_context]
[GLOBAL context_switch]
[GLOBAL __switch_task]
[extern current]

read_eip:
  pop eax
  jmp eax 


;
;struct Task{
;   
;  uint32_t esp0;
;  uint32_t eip;
;  uint32_t phy_dir;
;  uint32_t ebp;
;   ......
; }__attribute__((packed));

;    ret add                      esp
;    can add more field later                   
save_context:
  push ebx
  mov  ebx,[current]
  push eax
  mov  eax,cr3
  mov  [ebx+8] ,eax        ; phy_dir
  pop eax
  pop ebx
  ret 

;kparse_arg
;1: /x $esp = 0xc0efffe0
;2: /x $ebp = 0xc0effff8


;2: /x $ebp = 0xc0efffb8
;3: /x $esp = 0xc0efff90
;4: /x $eip = 0xc08023be
; current (0xc080d0a0) = {esp0 = 0x0, eip = 0x0, phy_dir = 0x100000, ebp = 0xc0efffb8, 
;           ptid = 0x0, tid = 0x0, state = 0x1}


;next_task (0xc080d0bc) = {esp0 = 0xbfffffe4, eip = 0x0, phy_dir = 0x1000000, ebp = 0x0, 
;                ptid = 0x0, tid = 0x1, state = 0x1}



;cdecl x86 system V ABI stack align

;https://levelup.gitconnected.com/x86-calling-conventions-a34812afe097
;https://apoorvaj.io/exploring-calling-conventions/
;    previous esp                 esp + 16
;    ptr to struct task *new_task esp + 12
;    ret add                      esp +8
;    ebx                          esp +4
;    eax                          esp

context_switch:
  ; save eip,esp
  push ebx
  push eax
  mov  ebx,[current]
  ; cdelc caller will restore esp, so save esp to the value it enter to context switch
  ; it's equal to 2*4 (push) + 4 (return eip) 
  mov  eax ,esp
  add  eax,12    
  mov  [ebx],eax

  mov  [ebx+12],ebp      ; ebp
  
  mov  eax,[esp+8]
  mov  [ebx+4] ,eax    ; eip

; for process terminated, it dosen't really need to save context, just call  __switch_task
__switch_task:
  ; current = new_task
  mov ebx,current
  mov eax,[esp+12]
  mov [ebx],eax
  ; context switch
  ; setup task initail task ebp to be the same as esp
  mov ebx,[current]
  mov esp, [ebx]
  mov ebp,[ebx+12]
  mov eax,[ebx+8]  
  mov cr3,eax

  pushf
  pop eax
  or eax,0x200
  push eax
  mov eax,0
  mov ax,cs
  push eax
  mov eax,[ebx+4]
  push eax
  iret