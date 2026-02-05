
# Nome kernel
KERNEL = kernel/kernel.bin

# Sorgente
C_SRCS = $(wildcard kernel/*.c)
ASM_SRCS =

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

# Compilatori
CC = gcc
LD = ld

# Regole
all: $(KERNEL)

$(KERNEL): $(C_SRCS)
	$(CC) $(CFLAGS) -c $(C_SRCS) -o kernel/kernel.o
	$(LD) $(LDFLAGS) kernel/kernel.o -o $(KERNEL)

clean:
	rm -f kernel/*.o $(KERNEL)
