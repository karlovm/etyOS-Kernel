org 0x7c00

jmp start

start:
    call displayWelcomeMessage   ; Display the welcome message
    call prompt

prompt:
    call clearBuffer             ; Clear the input buffer
    call nextLine                ; Move to the next line
    call requestInput            ; Request and capture user input
    jmp prompt                   ; Repeat prompt

clearBuffer:
    mov si, buffer               ; Point SI to the start of the buffer
    mov cx, 64                   ; Set CX to the size of the buffer (64 bytes)

clearLoop:
    mov byte [si], 0             ; Set the byte at SI to 0
    inc si                       ; Move to the next byte in the buffer
    loop clearLoop               ; Repeat until CX is zero
    ret                          ; Return from the function

buffer:
    times 64 db 0                ; 64-byte buffer to store user input

requestInput:
    xor bx, bx                   ; Reset BX (buffer index)
    mov si, buffer               ; Point SI to the buffer

getInput:
    mov ah, 0x00                 ; BIOS command to get a keystroke
    int 0x16                     ; Wait for keypress
    cmp al, 0x0D                 ; Compare AL with Enter key (0x0D)
    je commandExecutor           ; If Enter is pressed, execute command

    cmp al, 0x08                 ; Check if the key is Backspace (0x08)
    je handleBackspace           ; If so, handle the backspace

    mov [si], al                 ; Store the character in buffer
    inc si                       ; Move to the next byte in buffer
    cmp si, buffer + 64          ; Ensure we don't overflow the buffer
    je getInput                  ; Go back to get more input if buffer full

    ; Display typed character in real-time
    mov ah, 0x0E                 ; BIOS teletype service
    int 0x10                     ; Print the typed character
    jmp getInput                 ; Loop to get the next character

handleBackspace:
    cmp si, buffer               ; Check if we're at the start of the buffer
    je getInput                  ; If so, ignore backspace (no more chars to delete)

    dec si                       ; Move buffer index back
    mov byte [si], 0             ; Remove the last character in the buffer

    ; Move cursor back on screen and overwrite the character with a space
    mov ah, 0x0E                 ; BIOS teletype service
    mov al, 0x08                 ; Backspace character
    int 0x10                     ; Move cursor back
    mov al, ' '                  ; Space character
    int 0x10                     ; Overwrite with space
    mov al, 0x08                 ; Backspace again to move cursor back
    int 0x10                     ; Move cursor back
    jmp getInput                 ; Return to input loop

commandExecutor:
    mov si, buffer               ; Point SI to the start of the buffer
    ; Execute commands based on buffer content
    call compareCmd1             ; Check if command is "cmd1"
    jnz unknownCmd                ; If not, check for "cmd2"

    call cmd1                    ; Execute command 1 (start32 mode switch)
    ret

unknownCmd:
    call unknownCommand
    ret                          ; Return to the caller

compareCmd1:
    mov di, cmd1Str              ; Load command 1 string
    call saveRegs                ; Compare strings
    ret

saveRegs:
    pusha                        ; Save all registers

compareLoop:
    mov al, [si]                 ; Load byte from buffer
    cmp al, [di]                 ; Compare with byte from the command string
    jne notEqual                 ; If not equal, return with non-zero
    cmp al, 0                    ; If we reached the end of the string
    je stringsEqual              ; Strings are equal
    inc si                       ; Move to the next byte in buffer
    inc di                       ; Move to the next byte in the command string
    jmp compareLoop              ; Repeat comparison

stringsEqual:
    xor ax, ax                   ; Set zero flag (strings match)
    popa                         ; Restore registers
    ret

notEqual:
    mov ax, 1                    ; Set non-zero flag (strings don't match)
    popa                         ; Restore registers
    ret

cmd1:
    mov si, cmd1Message           ; Display command 2 message
    call printStrFromSi
    call nextLine
    call start32                 ; Switch to 32-bit mode
    ret

unknownCommand:
    call nextLine
    mov si, unknownCmdMessage      ; Display unknown command message
    call printStrFromSi
    ret

printStrFromSi:
    mov al, [si]                 ; Load the character pointed by SI
    cmp al, 0                    ; Check if it is the null terminator (0)
    je donePrinting              ; If so, exit the loop
    mov ah, 0x0E                 ; BIOS teletype service
    int 0x10                     ; Print the character
    inc si                       ; Move to the next character
    jmp printStrFromSi           ; Continue printing

donePrinting:
    ret                          ; Return from function

nextLine:
    mov ah, 0x0E                 ; BIOS function to print a character
    mov al, 0x0D                 ; Carriage return ('\r')
    int 0x10                     ; Print the carriage return
    mov ah, 0x0E                 ; BIOS function to print a character
    mov al, 0x0A                 ; Line feed ('\n')
    int 0x10                     ; Print the line feed
    ret                          ; Return from function

displayWelcomeMessage:
    push si                      ; Save SI register
    mov si, welcomeMessage        ; Load the welcome message address
    call printStrFromSi           ; Call the function to print it
    pop si                       ; Restore SI register
    ret                          ; Return from function

start32:
    cli                          ; Clear interrupts
    lgdt [gdt_descriptor]         ; Load GDT
    mov eax, cr0
    or eax, 0x1                  ; Set the PE (Protection Enable) bit
    mov cr0, eax
    jmp 08h:start32_protected     ; Far jump to flush the prefetch queue and enter protected mode

start32_protected:
    mov ax, 10h                  ; Set up data segment register
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call displayRainbowText       ; Call the rainbow text display routine

    ; Example: Infinite loop to halt execution
    jmp $

displayRainbowText:
    mov ebx, 0B8000h              ; Video memory base address
    mov cx, welcomeMessageLength  ; Length of the message to print
    mov si, rainbowMessage        ; Load the rainbow message address
    xor dx, dx                    ; Initialize color index
    call printRainbowFromSi
    ret

printRainbowFromSi:
    mov edi, colorTable            ; Point to the color table in 32-bit mode
    xor edx, edx                   ; Initialize the color index

printRainbowLoop:
    mov al, [esi]                  ; Load the character from the message
    cmp al, 0                      ; Check for the null terminator
    je doneRainbowPrinting         ; If it's the end of the string, stop
    mov ah, [edi + edx]            ; Load the color from the color table using 32-bit indexing
    mov word [es:ebx], ax          ; Store the character and color in video memory (2 bytes)
    inc esi                        ; Move to the next character
    add ebx, 2                     ; Move to the next video memory location (2 bytes per character/color)
    inc edx                        ; Increment the color index
    cmp edx, 7                     ; Check if we've reached the last color
    jne continueRainbow            ; If not, continue with the next color

    xor edx, edx                   ; Reset the color index to 0

continueRainbow:
    loop printRainbowLoop           ; Loop until the entire message is printed

doneRainbowPrinting:
    ret

colorTable:
    db 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7  ; Color values for rainbow effect


gdt:
    ; Null descriptor
    dq 0x0000000000000000
    ; Code segment descriptor
    dq 0x00cf9a000000ffff
    ; Data segment descriptor
    dq 0x00cf92000000ffff

gdt_descriptor:
    dw gdt_end - gdt - 1         ; Size of GDT
    dd gdt                       ; Address of GDT

gdt_end:

welcomeMessage:
    db "etyOS Bootscript loaded", 0  ; The message to display

cmd1Message:
    db "Entering 32-bit protected mode...", 0  ; Message for command 1

unknownCmdMessage:
    db "Unknown command.", 0         ; Message for unknown command

rainbowMessage:
    db "Welcome to 32-bit mode!", 0  ; Message for rainbow display
welcomeMessageLength equ $-rainbowMessage  ; Length of the rainbow message

cmd1Str:
    db "cmd1", 0                     ; Command 1 string


exit:
    jmp exit                     ; Infinite loop to halt execution

times 510-($-$$) db 0            ; Pad to 510 bytes
db 0x55, 0xaa                    ; Boot sector signature
