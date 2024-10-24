// kernel.cpp
/**********************************************************************
 * etyOS Kernel Core Implementation
 * Implements the core kernel functionality
 *********************************************************************/

#include "kernel.h"

/**
 * CommandProcessor Constructor
 * Initializes the command buffer and processor
 */
CommandProcessor::CommandProcessor(IOManager& io) : io_manager(io) {
    command_buffer[0] = '\0';
    command_length = 0;
}

/**
 * Read Command
 * Reads a command from keyboard input
 */
void CommandProcessor::read_command() {
    command_length = 0;
    int start_col = 2; // After prompt
    int current_row = io_manager.get_cursor_row();
    
    while (1) {
        // Wait for a key
        while ((IOManager::inb(keyboard::STATUS_PORT) & 0x01) == 0);

        unsigned char scancode = io_manager.read_scancode();
        char ascii_char = io_manager.scancode_to_ascii(scancode);
        
        if (ascii_char) {
            if (ascii_char == '\n') {  // Enter key
                command_buffer[command_length] = '\0';
                io_manager.print_newline();
                execute_command(command_buffer);
                return;
            }
            else if (ascii_char == '\b' && command_length > 0) {  // Backspace
                command_length--;
                int current_col = start_col + command_length;
                io_manager.print_char(' ', current_row, current_col, vga::color::WHITE_ON_BLACK);
                io_manager.set_cursor((current_row * vga::MAX_COLS + current_col) * 2);
            }
            else if (ascii_char >= ' ' && command_length < MAX_COMMAND_LENGTH - 1) {  // Printable chars
                int current_col = start_col + command_length;
                if (current_col < vga::MAX_COLS) {
                    command_buffer[command_length++] = ascii_char;
                    io_manager.print_char(ascii_char, current_row, current_col, vga::color::WHITE_ON_BLACK);
                }
            }
        }
    }
}

/**
 * String Equals
 * Compares two strings for equality
 */
bool CommandProcessor::str_equals(const char* str1, const char* str2) {
    int i = 0;
    while (str1[i] && str2[i] && str1[i] == str2[i]) i++;
    return str1[i] == str2[i];
}

/**
 * Handle Help Command
 * Displays available commands
 */
void CommandProcessor::handle_help_command() {
    io_manager.print_string("Available commands:", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
    io_manager.print_string("  clear - Clear the screen", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
    io_manager.print_string("  help  - Show this help message", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
    io_manager.print_string("  about - Show system information", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
}

/**
 * Handle About Command
 * Displays system information
 */
void CommandProcessor::handle_about_command() {
    io_manager.print_string("etyOS Kernel - A bare metal operating system", io_manager.get_cursor_row(), 0, vga::color::CYAN_ON_BLACK);
    io_manager.print_newline();
    io_manager.print_string("Version: 2.0", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
    io_manager.print_string("Created by Mikhail Karlov", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
io_manager.print_newline();
    io_manager.print_string("Build Date: October 2024", io_manager.get_cursor_row(), 0, vga::color::WHITE_ON_BLACK);
    io_manager.print_newline();
}

/**
 * Print Prompt
 * Displays the command prompt
 */
void CommandProcessor::print_prompt() {
    io_manager.print_string("> ", io_manager.get_cursor_row(), 0, vga::color::GREEN_ON_BLACK);
}

/**
 * Execute Command
 * Processes and executes the given command
 */
void CommandProcessor::execute_command(const char* command) {
    if (!command) {
        io_manager.print_string("Error: Invalid command", io_manager.get_cursor_row(), 0, vga::color::RED_ON_BLACK);
        io_manager.print_newline();
        print_prompt();
        return;
    }

    // Trim leading spaces
    while (*command == ' ') command++;

    // Handle empty command
    if (*command == '\0') {
        print_prompt();
        return;
    }

    if (str_equals(command, "clear")) {
        io_manager.clear_screen();
        print_prompt();
    }
    else if (str_equals(command, "help")) {
        handle_help_command();
        print_prompt();
    }
    else if (str_equals(command, "about")) {
        handle_about_command();
        print_prompt();
    }
    else if (str_equals(command, "shutdown")) {
        io_manager.print_string("Shutting down...", io_manager.get_cursor_row(), 0, vga::color::YELLOW_ON_BLACK);
        io_manager.print_newline();
        // TODO: Implement actual shutdown logic
    }
    else {
        io_manager.print_string("Unknown command: ", io_manager.get_cursor_row(), 0, vga::color::RED_ON_BLACK);
        io_manager.print_string(command, io_manager.get_cursor_row(), 16, vga::color::RED_ON_BLACK);
        io_manager.print_newline();
        io_manager.print_string("Type 'help' for available commands", io_manager.get_cursor_row(), 0, vga::color::YELLOW_ON_BLACK);
        io_manager.print_newline();
        print_prompt();
    }
}

/**
 * EtyKernel Constructor
 * Initializes the kernel and its components
 */
EtyKernel::EtyKernel() : cmd_processor(io_manager) {
    initialize();
}

/**
 * Initialize
 * Performs kernel initialization
 */
void EtyKernel::initialize() {
    io_manager.clear_screen();
    print_welcome_message();
}

/**
 * Print Welcome Message
 * Displays the kernel welcome message
 */
void EtyKernel::print_welcome_message() {
    io_manager.print_string("   etyOS Kernel v2.0 ", 0, 0, vga::color::CYAN_ON_BLACK);
    io_manager.print_string("Type 'help' for available commands", 4, 0, vga::color::YELLOW_ON_BLACK);
    io_manager.print_newline();
    cmd_processor.print_prompt();
}

/**
 * Run
 * Main kernel loop
 */
void EtyKernel::run() {
    while (true) {
        cmd_processor.read_command();
    }
}

/**
 * Kernel Entry Point
 */
extern "C" void main() {
    EtyKernel kernel;
    kernel.run();
}