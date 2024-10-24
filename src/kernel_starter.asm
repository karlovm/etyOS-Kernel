section .text
    [bits 32]
    [extern main]
    [global _start]

_start:
    ; Set up stack
    mov esp, stack_top         ; Point stack to our new stack space
    
    ; Try to call main
    call main
    test eax, eax             ; Check return value
    jnz .kernel_error         ; If non-zero, we have an error

    jmp $                     ; Infinite loop after successful main

.kernel_error:
    ; Use bootloader's crash handler at 0xB8000
    mov edi, 0xB8000          ; Video memory address
    mov ax, 0x4F00           ; White on red attribute (0x4F), null char (0x00)
    mov ecx, 80 * 25         ; Clear whole screen
    rep stosw                ; Fill screen with red background

    mov edi, 0xB8000         ; Reset video position
    mov esi, error_msg       ; Point to our error message
    mov ah, 0x4F            ; White text on red background

.print_loop:
    lodsb                    ; Load character
    test al, al             ; Check for end of string
    jz .error_halt          ; If done, halt
    mov [edi], ax          ; Write char with attribute
    add edi, 2             ; Next video position
    jmp .print_loop

.error_halt:
    cli                     ; Clear interrupts
    hlt                     ; Halt the CPU

section .data
    error_msg db 'KERNEL ERROR: System halted', 0

section .bss
    align 4                  ; Align to 4 bytes
    stack_bottom:
        resb 16384          ; Reserve 16KB for stack
    stack_top: