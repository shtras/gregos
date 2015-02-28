[bits 16]
print_string: ; parameter char* in bx
    pusha
    mov ah, 0xe
print_string_loop:
    mov al, [bx]
    cmp al, 0
    je print_string_end
    int 0x10
    inc bx
    jmp print_string_loop
print_string_end:
    popa
    ret


print_hex: ; parameter word in dx
    pusha
    mov bx, print_hex_inter
    add bx, 5
print_hex_loop:
    cmp dx, 0
    je print_hex_end
    mov ax, dx
    and ax, 0xf
    cmp ax, 9
    jg print_hex_gt_9
    add ax, '0'
    jmp print_hex_end_if
print_hex_gt_9:
    sub ax, 10
    add ax, 'a'
print_hex_end_if:
    mov [bx], al
    shr dx, 4
    dec bx
    jmp print_hex_loop
print_hex_end:
    mov bx, print_hex_inter
    call print_string
    popa
    ret
print_hex_inter:
    db '0x0000',0
    
disk_read:    
    push dx
    mov ah, 0x02
    mov al, dh ; sectors to read
    mov ch, 0 ; cylinder
    mov dh, 0 ; head
    mov cl, 2 ; start from sector
    int 0x13
    jc disk_read_error
    pop dx
    cmp dh, al
    jne disk_read_error1
    ret
disk_read_error:
    mov bx, disk_read_error_str
    call print_string
    jmp $
disk_read_error1:
    mov bx, disk_read_error_str1
    call print_string
    jmp $    
disk_read_error_str:
    db "Error reading from disk!", 0
    
disk_read_error_str1:
    db "Error reading from disk1!", 0

[bits 32]
VIDEO_MEM equ 0xb8000
W_B equ 0x0f

print_string_pm:
    pusha
    mov edx, VIDEO_MEM
print_string_pm_loop:
    mov al, [ebx]
    mov ah, W_B
    cmp al, 0
    je print_string_pm_end
    mov [edx], ax
    inc ebx
    add edx, 2
    jmp print_string_pm_loop
print_string_pm_end:
    popa
    ret