# Aurora OS v1.0🌌
A minimal 32-bit operating system written in C and Assembly for x86 architectures.

## 🚀 Key Features
* **Monolithic Kernel**: Direct management of hardware and VGA video memory.
* **Hybrid Interface**: Seamlessly switch between an interactive Terminal and a graphical Desktop.
* **Integrated Apps**: Calculator, File Manager, and an ASCII Image Viewer.
* **Input Systems**: Custom keyboard drivers and a functional mouse cursor (arrow-key controlled).
* **Real-Time Clock (RTC)**: Live clock synchronized with the motherboard's CMOS chip.
* **Bootloader**: Fully Multiboot compliant (GRUB ready).

## 🛠️ Build Instructions
To build Aurora OS, you will need `gcc`, `binutils` (ld, as), and `xorriso` (Linux environment recommended).

```bash
# 1. Assemble the bootloader
as --32 kernel/boot.s -o kernel/boot.o

# 2. Compile the kernel (bare-metal)
gcc -m32 -c kernel/kernel.c -o kernel/kernel.o -ffreestanding -O2 -fno-pic -fno-stack-protector

# 3. Link the files
ld -m elf_i386 -T linker.ld -o kernel/kernel.bin kernel/boot.o kernel/kernel.o

# 4. Generate the ISO
grub-mkrescue -o aurora.iso iso_root

#5. Running the OS
qemu-system-x86_64 -cdrom aurora.iso
