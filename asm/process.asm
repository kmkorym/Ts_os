[GLOBAL read_eip]
[GLOBAL __context_switch]
[extern current]
[extern exec_current]
[extern KSTACK_TOP]

read_eip:
  pop eax
  jmp eax 

;cdecl x86 system V ABI stack align

;https://levelup.gitconnected.com/x86-calling-conventions-a34812afe097
;https://apoorvaj.io/exploring-calling-conventions/

; gcc ( use cdecl ? ) don't restore eax , eax ?? 

;    ptr to struct task *new_task esp + 4
;    ret add                      esp 

; check cdecl will save eax,ecx for callee


; https://stackoverflow.com/questions/12315879/pushing-a-32-bit-register-onto-a-16-bit-stack
; push 16 bit registers still occupy 4 bytes ...



;;  save context + swtich address space +  restore previous context registers ..
__context_switch:
  ; check cdecl will save eax,ecx for callee  
  mov  ecx,[current]

  mov eax,cr3
  mov [ecx+4],eax
  ;; save new task ptr before change stack
  mov eax,[esp+4] 
  
 
  ;; save context , we only need to save esp and cr3
  ;; other registers will save above stack
  pusha
  mov [ecx], esp ; save stack of current

   ; current = new_task
  mov ecx,current
  mov [ecx],eax

  mov ecx,[ecx]
  mov esp,[ecx]   ; restore esp for new task
  mov eax,[ecx+4] 
  mov cr3,eax
  ;; remember to restore TSS.ESP0 field ...
  popa
  ret



