#!/bin/bash
function qemu
{
        /cygdrive/d/Program\ Files/qemu/qemu-system-x86_64.exe $1 &
}

nasm boot.asm -f bin -o boot.bin

if [[ $? -ne 0 ]] ; then
	exit 1
fi

nasm kernel_entry.asm -f elf32 -o kernel_entry.o

i686-elf-gcc -ffreestanding -c kernel.c -o kernel.o -m32 && \
i686-elf-ld -e start --oformat binary -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o -melf_i386 && \
#objcopy -O binary -j .text kernel.tmp kernel.bin && \
#rm kernel.o kernel.tmp


#./comp.sh kernel.c kernel.bin

if [[ $? -ne 0 ]] ; then
	exit 1
fi

cat boot.bin kernel.bin padding.bin > os_image

qemu os_image
