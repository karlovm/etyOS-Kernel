section .text
    [bits 32]
    [extern main]

    ; Set up a larger stack
    mov esp, stack_top     ; Point stack to our new stack space
    
    call main
    jmp $                  ; Infinite loop after main returns

section .bss
    align 4                ; Align to 4 bytes
    stack_bottom:
        resb 16384        ; Reserve 16KB for stack
    stack_top: