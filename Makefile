LD=i686-elf-ld
C=i686-elf-gcc
CC=i686-elf-g++
NASM=nasm
QEMU=/cygdrive/d/Program\ Files/qemu/qemu-system-i386.exe

CPP_SOURCE = $(wildcard kernel/*.cpp drivers/*.cpp utils/*.cpp)
HEADERS = $(wildcard kernel/*.h drivers/*.h utils/*.h)
ASM_SOURCE = $(wildcard boot/*.s kernel/*.s)
NASM_SOURCE = $(wildcard kernel/*.asm)

CPP_OBJS = ${CPP_SOURCE:.cpp=.o}
ASM_OBJS = ${ASM_SOURCE:.s=.o}
NASM_OBJS = ${NASM_SOURCE:.asm=.o}

CFLAGS = -ffreestanding -Wno-write-strings -fno-rtti -fno-exceptions -fno-threadsafe-statics -nostdlib  -DANOTHER_BRICK_IN_THE -Wall
LDFLAGS = -ffreestanding -nostdlib -lgcc
INC_FLAGS = -I. -Ikernel -Idrivers -Iutils

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI_OBJ=boot/crti.o
CRTN_OBJ=boot/crtn.o

#$^ - all dependancies
#$< - first dependancy
#$@ - target
run: build
	$(QEMU) -kernel kernel.bin

build: kernel.bin

rebuild: clean build
	
kernel.bin: $(NASM_OBJS) $(ASM_OBJS) $(CPP_OBJS)
	$(CC) -T linker.ld $(CRTI_OBJ) $(CRTBEGIN_OBJ) kernel/gdt_asm.o boot/boot.o $(NASM_OBJS) $(CPP_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ) -o $@ $(LDFLAGS)

#.asm.o: %.asm
%.o: %.asm
	nasm -f elf32 $< -o $@

#.s.o: %.s
%.o: %.s
	i686-elf-as -s $< -o $@
	
#.cpp.o: %.cpp ${HEADERS}
%.o: %.cpp ${HEADERS}
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@
	
clean:
	rm -f boot.bin kernel.bin
	find . -name "*.o" -exec rm -f {} \;