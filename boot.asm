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

kernel_crash:
    ; Set video mode back to text mode if needed
    mov ax, 0x0003
    int 0x10
    
    mov si, KERNEL_CRASH_MSG
    call print_error
    jmp $                          ; Halt system after crash

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
KERNEL_CRASH_MSG: db 'Kernel crash detected! System halted.', 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:                       ; CS should point to this
        dw 0xffff                   ; Segment limit first 0-15 bits
        dw 0x0                      ; Base first 0-15 bits
        db 0x0                      ; Base 16-23 bits
        db 0b10011010              ; Access byte
        db 0b11001111              ; High 4 bit flags and low 4 bit flags
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

    ; Clear screen
    mov edi, 0xB8000               ; Video memory address for text mode
    mov ecx, 80 * 25               ; 80x25 characters
    mov eax, 0x0720                ; 0x07 is attribute (light grey on black), 0x20 is space
    rep stosw                      ; Write to video memory, clear the screen

    ; Print success message
    mov edi, 0xB8000               ; Reset EDI to video memory start
    mov esi, msg                   ; Load address of the message
    call print_string

    ; Set up exception handlers
    mov eax, exception_handler
    mov [0x0], eax                 ; Division by zero handler
    mov [0x4], eax                 ; Debug exception
    mov [0x8], eax                 ; NMI interrupt
    mov [0xC], eax                 ; Breakpoint
    mov [0x10], eax                ; Overflow
    ; Add more exception handlers as needed

    ; Jump to kernel
    jmp KERNEL_LOCATION

; Exception handler in protected mode
exception_handler:
    mov edi, 0xB8000               ; Video memory address
    mov esi, crash_msg             ; Load crash message
    call print_string
    cli                            ; Disable interrupts
    hlt                            ; Halt the CPU

; Function to print a string in protected mode
print_string:
    mov ah, 0x07                   ; Attribute byte (light grey on black)
.next_char:
    lodsb                          ; Load next character from [esi] into AL
    test al, al                    ; Check if we hit the null terminator
    jz .done                       ; If zero, end of string
    stosw                          ; Store character and attribute in video memory
    jmp .next_char                 ; Repeat for next character
.done:
    ret

msg db "etyOS Kernel Starter is in 32-bit mode", 0
crash_msg db "KERNEL CRASH: System halted!", 0

; Boot sector padding
times 510-($-$$) db 0
dw 0xaa55