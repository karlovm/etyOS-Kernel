
// io.cpp
/**********************************************************************
 * IO Management Module Implementation
 * Implements all low-level input/output operations
 *********************************************************************/

#include "io.h"

/**
 * IOManager Constructor
 * Initializes the video memory pointer
 */
IOManager::IOManager() : video_memory((unsigned char*)vga::VIDEO_MEMORY) {}

/**
 * Clear Screen
 * Fills the entire screen with blank spaces
 */
void IOManager::clear_screen() {
    for (int row = 0; row < vga::MAX_ROWS; row++) {
        for (int col = 0; col < vga::MAX_COLS; col++) {
            print_char(' ', row, col, vga::color::WHITE_ON_BLACK);
        }
    }
    set_cursor(0);
}

/**
 * Print Character
 * Prints a single character at specified position with given attributes
 */
void IOManager::print_char(char character, int row, int col, char attribute_byte) {
    if (!attribute_byte) {
        attribute_byte = vga::color::WHITE_ON_BLACK;
    }
    
    if (row >= vga::MAX_ROWS) {
        scroll_screen();
        row = vga::MAX_ROWS - 1;
    }
    
    if (col >= vga::MAX_COLS) {
        row++;
        col = 0;
        if (row >= vga::MAX_ROWS) {
            scroll_screen();
            row = vga::MAX_ROWS - 1;
        }
    }
    
    int offset = (row * vga::MAX_COLS + col) * 2;
    video_memory[offset] = character;
    video_memory[offset + 1] = attribute_byte;
    set_cursor(offset + 2);
}

/**
 * Print String
 * Prints a null-terminated string at specified position
 */
void IOManager::print_string(const char* str, int row, int col, char attribute_byte) {
    int i = 0;
    while (str[i] != '\0') {
        print_char(str[i], row, col + i, attribute_byte);
        i++;
    }
}

/**
 * Print Newline
 * Moves cursor to the beginning of the next line
 */
void IOManager::print_newline() {
    int cursor_offset = get_cursor();
    int row = cursor_offset / (2 * vga::MAX_COLS);
    set_cursor((row + 1) * vga::MAX_COLS * 2);
}

/**
 * Scroll Screen
 * Moves all lines up by one and clears the bottom line
 */
void IOManager::scroll_screen() {
    // Move each line up
    for (int row = 1; row < vga::MAX_ROWS; row++) {
        for (int col = 0; col < vga::MAX_COLS; col++) {
            int current_pos = (row * vga::MAX_COLS + col) * 2;
            int previous_pos = ((row - 1) * vga::MAX_COLS + col) * 2;
            video_memory[previous_pos] = video_memory[current_pos];
            video_memory[previous_pos + 1] = video_memory[current_pos + 1];
        }
    }
    
    // Clear the last line
    int last_row = vga::MAX_ROWS - 1;
    for (int col = 0; col < vga::MAX_COLS; col++) {
        int pos = (last_row * vga::MAX_COLS + col) * 2;
        video_memory[pos] = ' ';
        video_memory[pos + 1] = vga::color::WHITE_ON_BLACK;
    }
}

/**
 * Get Cursor
 * Returns the current cursor position as an offset
 */
int IOManager::get_cursor() {
    outb(vga::REG_SCREEN_CTRL, 14);
    int offset = inb(vga::REG_SCREEN_DATA) << 8;
    outb(vga::REG_SCREEN_CTRL, 15);
    offset += inb(vga::REG_SCREEN_DATA);
    return offset * 2;
}

/**
 * Set Cursor
 * Sets the cursor position to the specified offset
 */
void IOManager::set_cursor(int offset) {
    offset /= 2;
    outb(vga::REG_SCREEN_CTRL, 14);
    outb(vga::REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    outb(vga::REG_SCREEN_CTRL, 15);
    outb(vga::REG_SCREEN_DATA, (unsigned char)(offset & 0xFF));
}

/**
 * Get Cursor Row
 * Returns the current cursor row
 */
int IOManager::get_cursor_row() {
    return (get_cursor() / 2) / vga::MAX_COLS;
}

/**
 * Get Cursor Column
 * Returns the current cursor column
 */
int IOManager::get_cursor_col() {
    return (get_cursor() / 2) % vga::MAX_COLS;
}

/**
 * Read from Port
 * Reads a byte from the specified I/O port
 */
unsigned char IOManager::inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

/**
 * Write to Port
 * Writes a byte to the specified I/O port
 */
void IOManager::outb(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

/**
 * Read Scancode
 * Reads a scancode from the keyboard controller
 */
unsigned char IOManager::read_scancode() {
    return inb(0x60);
}

/**
 * Scancode to ASCII
 * Converts a keyboard scancode to ASCII character
 */
char IOManager::scancode_to_ascii(unsigned char scancode) {
    // Simple scancode to ASCII conversion
    // This should be expanded for a full keyboard map
    static const char scancode_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_ascii)) {
        return scancode_ascii[scancode];
    }
    return 0;
}