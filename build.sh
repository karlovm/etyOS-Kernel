#!/bin/bash

###############################################################################
#  _________ __      ______  _____   _____ __                                  #
# |  ___|  _ \ \    / / ___|/ _ \ \ / / _ \/ ___|                             #
# | |_  | |_) \ \/\/ /\___ \ | | \ V / | | \___ \                             #
# |  _| |  __/ |\/| |  ___) | |_| || || |_| |___) |                          #
# |_|   |_|  |_|  |_| |____/ \___/ |_| \___/|____/                           #
#                                                                              #
# Build Script for etyOS Kernel                                                #
# Author: Mikhail Karlov                                                       #
# Version: 2.0                                                                 #
###############################################################################

# Configuration
COMPILER_PATH="/usr/local/i386elfgcc/bin"
BUILD_DIR="build"
BIN_DIR="bin"
SRC_DIR="src"
KERNEL_ENTRY=0x1000
MEMORY_SIZE="128M"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Error handling
set -e
trap 'echo -e "${RED}Error: Build failed${NC}" >&2' ERR

# Function to print status messages
print_status() {
    echo -e "${GREEN}[BUILD]${NC} $1"
}

# Function to print warnings
print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Create necessary directories
mkdir -p "$BUILD_DIR" "$BIN_DIR"

# Add compiler path to PATH
export PATH=$PATH:$COMPILER_PATH

# Function to check if required tools are installed
check_requirements() {
    print_status "Checking build requirements..."
    
    local REQUIRED_TOOLS=(
        "nasm"
        "i386-elf-gcc"
        "i386-elf-ld"
        "qemu-system-x86_64"
    )
    
    local MISSING_TOOLS=()
    
    for tool in "${REQUIRED_TOOLS[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            MISSING_TOOLS+=("$tool")
        fi
    done
    
    if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
        echo -e "${RED}Error: Missing required tools: ${MISSING_TOOLS[*]}${NC}"
        exit 1
    fi
}

# Clean build directories
clean() {
    print_status "Cleaning build directories..."
    rm -rf "$BUILD_DIR"/* "$BIN_DIR"/*
}

# Compile assembly files
compile_asm() {
    print_status "Compiling assembly files..."
    
    # Boot loader
    nasm "$SRC_DIR/boot.asm" -f bin -o "$BIN_DIR/boot.bin"
    
    # Kernel starter
    nasm "$SRC_DIR/kernel_starter.asm" -f elf -o "$BUILD_DIR/kernel_starter.o"
    
    # Zeroes file
    nasm "$SRC_DIR/zeroes.asm" -f bin -o "$BIN_DIR/zeroes.bin"
}

# Compile C++ files
compile_cpp() {
    print_status "Compiling C++ files..."
    
    # Common compiler flags
    local CPP_FLAGS="-ffreestanding -m32 -g -O2 -Wall -Wextra"
    
    # Compile each source file
    local CPP_FILES=(
        "kernel"
        "io"
        "system"
    )
    
    for file in "${CPP_FILES[@]}"; do
        print_status "  Compiling $file.cpp..."
        i386-elf-gcc $CPP_FLAGS -c "$SRC_DIR/$file.cpp" -o "$BUILD_DIR/$file.o"
    done
}

# Link object files
link_kernel() {
    print_status "Linking kernel..."
    
    local OBJECTS=(
        "$BUILD_DIR/kernel_starter.o"
        "$BUILD_DIR/kernel.o"
        "$BUILD_DIR/io.o"
        "$BUILD_DIR/system.o"
    )
    
    i386-elf-ld -o "$BIN_DIR/kernel.bin" \
        -Ttext $KERNEL_ENTRY \
        "${OBJECTS[@]}" \
        --oformat binary
}

# Create final disk image
create_disk_image() {
    print_status "Creating disk image..."
    cat "$BIN_DIR/boot.bin" \
        "$BIN_DIR/kernel.bin" \
        "$BIN_DIR/zeroes.bin" > "$BIN_DIR/etyOS.bin"
    
    # Calculate image size
    local image_size=$(stat -f %z "$BIN_DIR/etyOS.bin")
    print_status "Disk image size: $image_size bytes"
}

# Run in QEMU
run_qemu() {
    print_status "Starting QEMU..."
    qemu-system-x86_64 \
        -drive format=raw,file="$BIN_DIR/etyOS.bin",index=0,if=floppy \
        -m $MEMORY_SIZE \
        -monitor stdio \
        -serial file:serial.log
}

# Main build process
main() {
    local start_time=$(date +%s)
    
    print_status "Starting etyOS build process..."
    
    check_requirements
    clean
    compile_asm
    compile_cpp
    link_kernel
    create_disk_image
    
    local end_time=$(date +%s)
    local build_time=$((end_time - start_time))
    
    print_status "Build completed in $build_time seconds"
    
    # Ask to run QEMU
    read -p "Do you want to run etyOS in QEMU? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        run_qemu
    fi
}

# Parse command line arguments
case "${1:-build}" in
    clean)
        clean
        ;;
    build)
        main
        ;;
    run)
        run_qemu
        ;;
    *)
        echo "Usage: $0 {build|clean|run}"
        echo "  build: Build etyOS (default)"
        echo "  clean: Clean build directories"
        echo "  run: Run etyOS in QEMU"
        exit 1
        ;;
esac