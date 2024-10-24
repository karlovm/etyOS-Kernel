// Add new header file: system.h
#ifndef ETY_SYSTEM_H
#define ETY_SYSTEM_H

/**********************************************************************
 * System Management Module Header
 * Defines system-level operations and utilities
 *********************************************************************/

class SystemManager {
public:
    static void halt();
    static void reboot();
    static void panic(const char* message);
    static void enable_interrupts();
    static void disable_interrupts();
    
private:
    static void wait_for_io();
};

#endif // ETY_SYSTEM_H

