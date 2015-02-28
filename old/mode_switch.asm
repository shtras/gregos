; switching to 32 bit mode
[bits 16]
switch_to_32:
    cli                 ; disable interrupts
    lgdt [gdt_descriptor]
    mov eax, cr0        ; switch on the first bit of a cr (damned little endian)
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:start_protected_mode
    
[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebp, 0x90000
    mov esp, ebp
    
    call start_pm
;gdt
gdt_start:
gdt_null:
    dd 0    ; leading zero
    dd 0
    
gdt_code:
    dw 0xffff ; limit 0-15
    dw 0      ; base 0-15
    db 0      ; base 16-23
    db 10011010b ; 1st flags, type flags
    db 11001111b ; 2nd flags, limit 16-19
    db 0      ; base 24-31
    
gdt_data:
    dw 0xffff ; limit 0-15
    dw 0      ; base 0-15
    db 0      ; base 16-23
    db 10010010b ; 1st flags, type flags
    db 11001111b ; 2nd flags, limit 16-19
    db 0      ; base 24-31
    
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start