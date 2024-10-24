#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Text color attributes (Foreground | Background)
#define WHITE_ON_BLACK 0x0F
#define RED_ON_BLACK 0x04
#define GREEN_ON_BLACK 0x02
#define YELLOW_ON_BLACK 0x0E
#define CYAN_ON_BLACK 0x03


#define KB_STATUS_PORT 0x64

// VGA I/O ports and control characters
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5
#define BACKSPACE 0x08
#define ENTER 0x0D
#define MAX_COMMAND_LENGTH 256

// Global variables
char command_buffer[MAX_COMMAND_LENGTH];
int command_length = 0;

// Function prototypes
void print_char(char character, int row, int col, char attribute_byte);
void print_string(const char* str, int row, int col, char attribute_byte);
void clear_screen();
int get_cursor();
void set_cursor(int offset);
void scroll_screen();
void read_command();
void execute_command(const char* command);
void print_newline();

// External functions
extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);
extern unsigned char read_scancode();
extern char scancode_to_ascii(unsigned char scancode);

// Get cursor row from offset
int get_cursor_row() {
    return (get_cursor() / 2) / MAX_COLS;
}

// Get cursor column from offset
int get_cursor_col() {
    return (get_cursor() / 2) % MAX_COLS;
}

// Scroll the screen up one line
void scroll_screen() {
    unsigned char* video_memory = (unsigned char*)VIDEO_MEMORY;
    
    // Move each line up
    for (int row = 1; row < MAX_ROWS; row++) {
        for (int col = 0; col < MAX_COLS; col++) {
            int current_pos = (row * MAX_COLS + col) * 2;
            int previous_pos = ((row - 1) * MAX_COLS + col) * 2;
            video_memory[previous_pos] = video_memory[current_pos];
            video_memory[previous_pos + 1] = video_memory[current_pos + 1];
        }
    }
    
    // Clear the last line
    int last_row = MAX_ROWS - 1;
    for (int col = 0; col < MAX_COLS; col++) {
        int pos = (last_row * MAX_COLS + col) * 2;
        video_memory[pos] = ' ';
        video_memory[pos + 1] = WHITE_ON_BLACK;
    }
}

// Print a single character
void print_char(char character, int row, int col, char attribute_byte) {
    if (!attribute_byte) {
        attribute_byte = WHITE_ON_BLACK;
    }
    
    if (row >= MAX_ROWS) {
        scroll_screen();
        row = MAX_ROWS - 1;
    }
    
    if (col >= MAX_COLS) {
        row++;
        col = 0;
        if (row >= MAX_ROWS) {
            scroll_screen();
            row = MAX_ROWS - 1;
        }
    }
    
    unsigned char* video_memory = (unsigned char*)VIDEO_MEMORY;
    int offset = (row * MAX_COLS + col) * 2;
    video_memory[offset] = character;
    video_memory[offset + 1] = attribute_byte;
    set_cursor(offset + 2);
}

// Print a string
void print_string(const char* str, int row, int col, char attribute_byte) {
    int i = 0;
    while (str[i] != '\0') {
        print_char(str[i], row, col + i, attribute_byte);
        i++;
    }
}

// Print newline
void print_newline() {
    int cursor_offset = get_cursor();
    int row = cursor_offset / (2 * MAX_COLS);
    set_cursor((row + 1) * MAX_COLS * 2);
}

// Modified read_command to handle keyboard input more robustly
void read_command() {
    command_length = 0;
    int start_col = 2; // After prompt
    int current_row = get_cursor_row();
    
    while (1) {
        // Wait for a key to be available
        while ((inb(KB_STATUS_PORT) & 0x01) == 0) {
            // Could add a yield or small delay here if needed
        }

        unsigned char scancode = read_scancode();
        char ascii_char = scancode_to_ascii(scancode);
        
        // Only process actual character input
        if (ascii_char) {
            if (ascii_char == '\n') {  // Enter key
                command_buffer[command_length] = '\0';
                print_newline();
                execute_command(command_buffer);
                return;
            }
            else if (ascii_char == '\b' && command_length > 0) {  // Backspace
                command_length--;
                int current_col = start_col + command_length;
                print_char(' ', current_row, current_col, WHITE_ON_BLACK);
                set_cursor((current_row * MAX_COLS + current_col) * 2);
            }
            else if (ascii_char >= ' ' && command_length < MAX_COMMAND_LENGTH - 1) {  // Printable characters
                int current_col = start_col + command_length;
                if (current_col < MAX_COLS) {
                    command_buffer[command_length++] = ascii_char;
                    print_char(ascii_char, current_row, current_col, WHITE_ON_BLACK);
                }
            }
        }
    }
}

// Compare strings
bool str_equals(const char* str1, const char* str2) {
    int i = 0;
    while (str1[i] && str2[i] && str1[i] == str2[i]) i++;
    return str1[i] == str2[i];
}

// Modified execute_command with error handling
void execute_command(const char* command) {
    if (!command) {
        print_string("Error: Invalid command", get_cursor_row(), 0, RED_ON_BLACK);
      
        return;
    }

    // Trim leading spaces
    while (*command == ' ') command++;

    // Handle empty command
    if (*command == '\0') {
       
        print_string("> ", get_cursor_row(), 0, GREEN_ON_BLACK);
        return;
    }

    if (str_equals(command, "clear")) {
        clear_screen();
    }
    else if (str_equals(command, "help")) {
        print_string("Available commands:", get_cursor_row(), 0, WHITE_ON_BLACK);
        print_newline();
        print_string("  clear - Clear the screen", get_cursor_row(), 0, WHITE_ON_BLACK);
        print_newline();
        print_string("  help  - Show this help message", get_cursor_row(), 0, WHITE_ON_BLACK);
        print_newline();
        print_string("  about - Show system information", get_cursor_row(), 0, WHITE_ON_BLACK);
        print_newline();
    }
    else if (str_equals(command, "about")) {
        print_string("etyOS Kernel - A bare metal operating system", get_cursor_row(), 0, CYAN_ON_BLACK);
        print_newline();
        print_string("Created by Mikhail Karlov", get_cursor_row(), 0, WHITE_ON_BLACK);
        print_newline();
    }
    else {
        print_string("Unknown command: ", get_cursor_row(), 0, RED_ON_BLACK);
        print_string(command, get_cursor_row(), 16, RED_ON_BLACK);
        print_newline();
        print_string("Type 'help' for available commands", get_cursor_row(), 0, YELLOW_ON_BLACK);
        print_newline();
    }

    print_string("> ", get_cursor_row(), 0, GREEN_ON_BLACK);
}
// Clear screen
void clear_screen() {
    for (int row = 0; row < MAX_ROWS; row++) {
        for (int col = 0; col < MAX_COLS; col++) {
            print_char(' ', row, col, WHITE_ON_BLACK);
        }
    }
    set_cursor(0);
    print_string("etyOS Kernel v.1.0 / Mikhail Karlov", 0, 0, CYAN_ON_BLACK);
   
    print_string("> ", 1, 0, GREEN_ON_BLACK);
}

// Get cursor position
int get_cursor() {
    outb(REG_SCREEN_CTRL, 14);
    int offset = inb(REG_SCREEN_DATA) << 8;
    outb(REG_SCREEN_CTRL, 15);
    offset += inb(REG_SCREEN_DATA);
    return offset * 2;
}

// Set cursor position
void set_cursor(int offset) {
    offset /= 2;
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, (unsigned char)(offset & 0xFF));
}

// Modified main function with initialization check
extern "C" void main() {
    // Initialize video memory
    volatile unsigned char* video_memory = (unsigned char*)VIDEO_MEMORY;
    if (!video_memory) {
        return;  // Exit if video memory isn't accessible
    }

    clear_screen();
  
   


    int io = 10 /0;
    print_string("> ", get_cursor_row(), 0, GREEN_ON_BLACK);

    while (1) {
        read_command();
    }
}