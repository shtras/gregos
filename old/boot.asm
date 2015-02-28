[org 0x7c00]
KERNEL_OFFSET equ 0x1000
[bits 16]
jmp main
main:
    mov [boot_drive], dl
    mov sp, 0x8000
    mov bp, sp

    ;mov bx, 0x9000
    ;mov dh, 2
    ;mov dl, [boot_drive]
    ;call disk_read
    ;
    ;mov dx, [0x9000]
    ;call print_hex
    ;
    ;mov dx, [0x9000 + 512]
    ;call print_hex
    
    call load_kernel
    
    mov bx, str_initializing_pm
    call print_string
    
    call switch_to_32
    
    jmp $

load_kernel:
    mov bx, str_loading_kernel
    call print_string
    
    mov bx, KERNEL_OFFSET
    mov dh, 15  ; load 15 sectors
    call disk_read
    ret
    
str_loading_kernel:
    db 'Loading kernel... ', 0

str_initializing_pm:
    db 'Initializing protected mode... ',0
    
%include "stdio.asm"
    
boot_drive:
    db 0

%include "mode_switch.asm"
[bits 32]
; Protected mode starts here
start_pm:
    call KERNEL_OFFSET
    jmp $

times 510-($-$$) db 0

dw 0xaa55
