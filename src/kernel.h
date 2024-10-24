// kernel.h
#ifndef ETY_KERNEL_H
#define ETY_KERNEL_H

/**********************************************************************
 * etyOS Kernel Core Header
 * Defines the core kernel classes and interfaces
 *********************************************************************/

#include "io.h"

class CommandProcessor {
public:
    static constexpr int MAX_COMMAND_LENGTH = 256;
    
    CommandProcessor(IOManager& io);
    void read_command();
    void execute_command(const char* command);
      void print_prompt();
    
private:
    char command_buffer[MAX_COMMAND_LENGTH];
    int command_length;
    IOManager& io_manager;
    
    bool str_equals(const char* str1, const char* str2);
    void handle_help_command();
    void handle_about_command();
  
};

class EtyKernel {
public:
    EtyKernel();
    void run();
    
private:
    IOManager io_manager;
    CommandProcessor cmd_processor;
    
    void initialize();
    void print_welcome_message();
};

#endif // ETY_KERNEL_H