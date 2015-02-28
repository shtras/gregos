#!/bin/bash

gcc -ffreestanding -c $1 -o temp.o -m32 && \
ld -r -o temp.tmp -Ttext 0x1000 temp.o -mi386pe && \
objcopy -O binary -j .text temp.tmp $2 && \
rm temp.o temp.tmp
