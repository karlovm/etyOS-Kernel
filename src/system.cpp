/**********************************************************************
 * System Management Module Implementation
 * Implements system-level operations
 *********************************************************************/

#include "system.h"
#include "io.h"
#include <stdint.h>  // Changed to C-style header for bare metal environment

/**
 * Halt
 * Stops the CPU
 */
void SystemManager::halt() {
    asm volatile("hlt");
}

/**
 * Reboot
 * Reboots the system using keyboard controller
 */
void SystemManager::reboot() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = IOManager::inb(0x64);
    IOManager::outb(0x64, 0xFE);
    halt();
}

/**
 * Panic
 * Handles kernel panic situations
 */
void SystemManager::panic(const char* message) {
    disable_interrupts();
    IOManager io;
    io.clear_screen();
    io.print_string("KERNEL PANIC", 0, 0, vga::color::RED_ON_BLACK);
    io.print_newline();
    io.print_string(message, io.get_cursor_row(), 0, vga::color::RED_ON_BLACK);
    halt();
}

/**
 * Enable Interrupts
 * Enables CPU interrupts
 */
void SystemManager::enable_interrupts() {
    asm volatile("sti");
}

/**
 * Disable Interrupts
 * Disables CPU interrupts
 */
void SystemManager::disable_interrupts() {
    asm volatile("cli");
}

/**
 * Wait for I/O
 * Short delay for I/O operations
 */
void SystemManager::wait_for_io() {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}