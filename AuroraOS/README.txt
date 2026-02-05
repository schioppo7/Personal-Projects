# Aurora OS v1.8 🌌
Un sistema operativo minimale a 32-bit scritto in C e Assembly per architetture x86.

## 🚀 Caratteristiche principali
* **Kernel Monolitico**: Gestione diretta dell'hardware e della memoria video VGA.
* **Hybrid Interface**: Passa fluidamente da un Terminale interattivo a un Desktop grafico.
* **App Integrate**: Calcolatrice, File Manager e Image Viewer con rendering ASCII.
* **Mouse & Keyboard**: Cursore gestibile tramite frecce direzionali e input da tastiera.
* **Real-Time Clock**: Orologio sincronizzato con il chip CMOS della scheda madre.
* **Bootloader**: Compatibile con GRUB (Multiboot).

## 🛠️ Come Compilare
Assicurati di avere `gcc`, `binutils` (ld, as) e `xorriso` installati (Linux consigliato).

```bash
# Compilazione e creazione ISO
as --32 kernel/boot.s -o kernel/boot.o
gcc -m32 -c kernel/kernel.c -o kernel/kernel.o -ffreestanding -O2 -fno-pic
ld -m elf_i386 -T linker.ld -o kernel/kernel.bin kernel/boot.o kernel/kernel.o
grub-mkrescue -o aurora.iso iso_root