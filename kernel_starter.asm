section .text
    [bits 32]
    [extern main]
    global _start

_start:
    ; Set up exception handlers
    cli                         ; Disable interrupts while setting up IDT
    call setup_idt
    
    ; Set up a larger stack
    mov esp, stack_top          ; Point stack to our new stack space
    
    ; Enable interrupts
    sti
    
    ; Call main function
    call main
    
    ; If main returns, go into infinite loop
    cli                         ; Disable interrupts
    hlt                         ; Halt the CPU
    jmp $                       ; Infinite loop after main returns

; Exception handler
exception_handler:
    cli                         ; Disable interrupts
    pusha                       ; Save all registers
    
    ; Print error message to video memory
    mov edi, 0xB8000           ; Video memory address
    mov esi, exception_msg
    mov ah, 0x4F               ; White text on red background
    
.print_loop:
    lodsb                      ; Load next character
    test al, al
    jz .end_print
    mov [edi], ax             ; Write character and attribute
    add edi, 2
    jmp .print_loop
    
.end_print:
    popa                       ; Restore registers
    
    ; Halt the system
    cli
    hlt
    jmp $

; Set up Interrupt Descriptor Table
setup_idt:
    ; Save registers
    pusha
    
    ; Set up exception handlers in IDT
    mov edi, 0                 ; IDT starts at address 0
    mov edx, exception_handler ; Handler address
    mov cx, 32                 ; Set up first 32 entries (CPU exceptions)
    
.idt_loop:
    ; Create IDT entry
    mov eax, edx              ; Handler address
    mov word [edi], ax        ; Handler address low bits
    mov word [edi + 2], 0x08  ; Kernel code segment
    mov byte [edi + 4], 0     ; Reserved
    mov byte [edi + 5], 0x8E  ; Present, Ring 0, 32-bit Interrupt Gate
    shr eax, 16
    mov word [edi + 6], ax    ; Handler address high bits
    
    add edi, 8                ; Move to next IDT entry
    loop .idt_loop
    
    ; Load IDT
    lidt [idt_descriptor]
    
    popa
    ret

section .data
    exception_msg db "etyKernel Exception! System Halted. Reboot your PC", 0
    
    ; IDT descriptor
    idt_descriptor:
        dw 256 * 8 - 1        ; Size of IDT (256 entries * 8 bytes - 1)
        dd 0                  ; Start address of IDT (0 in this case)

section .bss
    align 4                   ; Align to 4 bytes
    stack_bottom:
        resb 16384           ; Reserve 16KB for stack
    stack_top: