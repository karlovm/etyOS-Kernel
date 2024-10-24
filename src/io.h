
// io.h
#ifndef ETY_IO_H
#define ETY_IO_H

/**********************************************************************
 * IO Management Module Header
 * Defines the interface for all input/output operations in etyOS
 *********************************************************************/

// VGA Hardware Interface Constants
namespace vga {
    constexpr unsigned int VIDEO_MEMORY = 0xB8000;
    constexpr int MAX_ROWS = 25;
    constexpr int MAX_COLS = 80;
    
    // Text Colors and Attributes
    namespace color {
        constexpr char WHITE_ON_BLACK = 0x0F;
        constexpr char RED_ON_BLACK = 0x04;
        constexpr char GREEN_ON_BLACK = 0x02;
        constexpr char YELLOW_ON_BLACK = 0x0E;
        constexpr char CYAN_ON_BLACK = 0x03;
    }
    
    // VGA Control Registers
    constexpr unsigned short REG_SCREEN_CTRL = 0x3D4;
    constexpr unsigned short REG_SCREEN_DATA = 0x3D5;
}

// Keyboard Interface Constants
namespace keyboard {
    constexpr unsigned short STATUS_PORT = 0x64;
    constexpr char BACKSPACE = 0x08;
    constexpr char ENTER = 0x0D;
}

class IOManager {
public:
    IOManager();
    
    // Screen Management
    void clear_screen();
    void print_char(char character, int row, int col, char attribute_byte);
    void print_string(const char* str, int row, int col, char attribute_byte);
    void print_newline();
    void scroll_screen();
    
    // Cursor Management
    int get_cursor();
    void set_cursor(int offset);
    int get_cursor_row();
    int get_cursor_col();
    
    // Port I/O Operations
    static unsigned char inb(unsigned short port);
    static void outb(unsigned short port, unsigned char data);
    
    // Keyboard Operations
    unsigned char read_scancode();
    char scancode_to_ascii(unsigned char scancode);

private:
    unsigned char* video_memory;
};

#endif // ETY_IO_H