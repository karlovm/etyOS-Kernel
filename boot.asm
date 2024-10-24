[org 0x7c00]                        
KERNEL_LOCATION equ 0x1000
                                    
mov [BOOT_DISK], dl                 

; Initialize segment registers and stack                                    
xor ax, ax                          
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp

; Load the kernel
mov bx, KERNEL_LOCATION
mov dh, 32                          ; Number of sectors to read

mov ah, 0x02
mov al, dh 
mov ch, 0x00
mov dh, 0x00
mov cl, 0x02
mov dl, [BOOT_DISK]
int 0x13
jc kernel_load_error                ; If carry flag is set, there was an error

; Compare sectors read
cmp al, 32                          ; Check if we read all requested sectors
jne kernel_load_error

; Set video mode                                    
mov ah, 0x0
mov al, 0x3
int 0x10                            ; text mode

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

kernel_load_error:
    mov si, KERNEL_ERROR_MSG
    call print_error
    jmp $

; Function to print error message
print_error:
    mov ah, 0x0e                    ; BIOS teletype output
.loop:
    lodsb                           ; Load next character in AL
    test al, al                     ; Check if end of string (0)
    jz .done                        ; If zero, we're done
    int 0x10                        ; Print character
    jmp .loop
.done:
    ret

BOOT_DISK: db 0
KERNEL_ERROR_MSG: db 'Failed to load kernel!', 0
KERNEL_CRASH_MSG: db 'KERNEL CRASH: System halted!', 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:                       ; CS should point to this
        dw 0xffff                   ; Segment limit first 0-15 bits
        dw 0x0                      ; Base first 0-15 bits
        db 0x0                      ; Base 16-23 bits
        db 0b10011010               ; Access byte
        db 0b11001111               ; High 4 bit flags and low 4 bit flags
        db 0x0                      ; Base 24-31 bits

    GDT_data:                       ; DS, SS, ES, FS, GS
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1     ; Size (16 bit)
    dd GDT_start                    ; Start address (32 bit)

[bits 32]
start_protected_mode:
    ; Initialize segment registers with data segment
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Setup 32-bit stack
    mov ebp, 0x90000
    mov esp, ebp

    ; Set up IDT entries for exceptions
    mov edi, 0x0                    ; First IDT entry
    mov eax, exception_handler      ; Our handler address
    mov cx, 32                      ; Set up handlers for first 32 exceptions
    
.setup_idt_loop:
    mov word [edi], ax             ; Handler address low word
    mov word [edi + 2], CODE_SEG   ; Kernel code segment
    mov word [edi + 4], 0x8E00     ; Type: 32-bit interrupt gate
    shr eax, 16
    mov word [edi + 6], ax         ; Handler address high word
    add edi, 8                     ; Next IDT entry
    loop .setup_idt_loop

    ; Clear screen
    mov edi, 0xB8000               ; Video memory address
    mov ecx, 80 * 25               ; 80x25 characters
    mov ax, 0x0720                 ; Normal attribute (gray on black)
    rep stosw                      ; Clear screen

    ; Print boot message
    mov edi, 0xB8000
    mov esi, boot_msg
    call print_string_pm

    ; Jump to kernel
    jmp KERNEL_LOCATION

; Exception handler
exception_handler:
    ; Save all registers
    pushad
    
    ; Clear screen with error colors (red background)
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x4F20                 ; White on red, space
    rep stosw
    
    ; Print error message
    mov edi, 0xB8000
    mov esi, crash_msg
    mov ah, 0x4F                   ; White on red attribute
    call print_string_pm
    
    ; Halt the system
    cli
    hlt
    jmp $                          ; In case of NMI

; Protected mode string printing
print_string_pm:
.loop:
    lodsb                          ; Load next character
    test al, al                    ; Check for null terminator
    jz .done
    mov [edi], ax                  ; Store character and attribute
    add edi, 2                     ; Next character position
    jmp .loop
.done:
    ret

boot_msg db "etyOS Kernel Starter is in 32-bit mode", 0
crash_msg db "etyOS KERNEL CRASH DETECTED! System halted.", 0

times 510-($-$$) db 0
dw 0xaa55