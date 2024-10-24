export PATH=$PATH:/usr/local/i386elfgcc/bin

# Assemble boot and kernel starter
nasm "boot.asm" -f bin -o "bin/boot.bin"
nasm "kernel_starter.asm" -f elf -o "bin/kernel_starter.o"

# Compile kernel.cpp and io.cpp
i386-elf-gcc -ffreestanding -m32 -g -c "kernel.cpp" -o "bin/kernel.o"
i386-elf-gcc -ffreestanding -m32 -g -c "io.cpp" -o "bin/io.o"

# Assemble zeroes
nasm "zeroes.asm" -f bin -o "bin/zeroes.bin"

# Link kernel with starter and io object file
i386-elf-ld -o "bin/kernel.bin" -Ttext 0x1000 "bin/kernel_starter.o" "bin/kernel.o" "bin/io.o" --oformat binary

# Combine all binaries into a single kernel image
cat "bin/boot.bin" "bin/kernel.bin" "bin/zeroes.bin" > "bin/etyOS.bin"

# Run the kernel in QEMU
qemu-system-x86_64 -drive format=raw,file="bin/etyOS.bin",index=0,if=floppy, -m 128M
