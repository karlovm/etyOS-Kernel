// io.cpp

// Read a byte from the specified port
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

// Write a byte to the specified port
void outb(unsigned short port, unsigned char data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
}

// Wait for an I/O operation to complete (small delay)
void io_wait() {
    // Port 0x80 is used for 'wasting time' on x86
    __asm__ __volatile__("outb %%al, $0x80" : : "a"(0));
}

// Function to check if the keyboard is ready for reading
unsigned char keyboard_data_ready() {
    return inb(0x64) & 1;  // Check bit 0 (output buffer status)
}

// Read a byte from the keyboard input port (0x60)
unsigned char read_scancode() {
    // Wait until the keyboard has data ready
    while (!keyboard_data_ready()) {
        // Busy-wait until the keyboard has data
    }
    return inb(0x60);  // Read the scancode from port 0x60
}
// Define the lookup table size to cover all possible scancodes
#define SCANCODE_TABLE_SIZE 128

// Create a static lookup table
static const char scancode_to_ascii_table[SCANCODE_TABLE_SIZE] = {
    0,    // 0x00
    0,    // 0x01 - ESC
    '1',  // 0x02
    '2',  // 0x03
    '3',  // 0x04
    '4',  // 0x05
    '5',  // 0x06
    '6',  // 0x07
    '7',  // 0x08
    '8',  // 0x09
    '9',  // 0x0A
    '0',  // 0x0B
    '-',  // 0x0C
    '=',  // 0x0D
    '\b', // 0x0E - Backspace
    '\t', // 0x0F - Tab
    'q',  // 0x10
    'w',  // 0x11
    'e',  // 0x12
    'r',  // 0x13
    't',  // 0x14
    'y',  // 0x15
    'u',  // 0x16
    'i',  // 0x17
    'o',  // 0x18
    'p',  // 0x19
    '[',  // 0x1A
    ']',  // 0x1B
    '\n', // 0x1C - Enter
    0,    // 0x1D - Left Control
    'a',  // 0x1E
    's',  // 0x1F
    'd',  // 0x20
    'f',  // 0x21
    'g',  // 0x22
    'h',  // 0x23
    'j',  // 0x24
    'k',  // 0x25
    'l',  // 0x26
    ';',  // 0x27
    '\'', // 0x28
    '`',  // 0x29
    0,    // 0x2A - Left Shift
    '\\', // 0x2B
    'z',  // 0x2C
    'x',  // 0x2D
    'c',  // 0x2E
    'v',  // 0x2F
    'b',  // 0x30
    'n',  // 0x31
    'm',  // 0x32
    ',',  // 0x33
    '.',  // 0x34
    '/',  // 0x35
    0,    // 0x36 - Right Shift
    '*',  // 0x37
    0,    // 0x38 - Left Alt
    ' ',  // 0x39 - Space
};

// Efficient scancode to ASCII conversion function
char scancode_to_ascii(unsigned char scancode) {
    // Check if scancode is within bounds and return the mapped character
    if (scancode < SCANCODE_TABLE_SIZE) {
        return scancode_to_ascii_table[scancode];
    }
    return 0;  // Return 0 for unmapped scancodes
}
