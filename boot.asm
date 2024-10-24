[org 0x7c00]                        
KERNEL_LOCATION equ 0x1000
                                    

mov [BOOT_DISK], dl                 

                                    
xor ax, ax                          
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION
mov dh, 32

mov ah, 0x02
mov al, dh 
mov ch, 0x00
mov dh, 0x00
mov cl, 0x02
mov dl, [BOOT_DISK]
int 0x13                ; no error management, do your homework!

                                    
mov ah, 0x0
mov al, 0x3
int 0x10                ; text mode


CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

jmp $
                                    
BOOT_DISK: db 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start


[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov ebp, 0x90000		; 32-bit stack base pointer
	mov esp, ebp

	; Clear screen
    mov edi, 0xB8000       ; Video memory address for text mode
    mov ecx, 80 * 25       ; 80x25 characters
    mov eax, 0x0720        ; 0x07 is the attribute (light grey on black), 0x20 is the space character
    rep stosw              ; Write to video memory, clear the screen

	; Print "etyOS Kernel Starter is in 32-bit mode"
    mov edi, 0xB8000       ; Reset EDI to video memory start
    mov esi, msg           ; Load address of the message
    call print_string

	; Jump to kernel start
    jmp KERNEL_LOCATION

; Function to print a string to the screen
print_string:
    mov ah, 0x07           ; Attribute byte (light grey on black)
.next_char:
    lodsb                  ; Load next character from [esi] into AL
    test al, al            ; Check if we hit the null terminator
    jz .done               ; If zero, end of string
    stosw                  ; Store character and attribute in video memory
    jmp .next_char         ; Repeat for next character
.done:
    ret                    ; Return from the function

msg db "etyOS Kernel Starter is in 32-bit mode", 0

times 510-($-$$) db 0
dw 0xaa55
