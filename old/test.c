void start()
{
    char* mem = (char*)0xb8000;
    *mem = 'X';
}
//ld -r -o test.tmp -Ttext 0x0 test.o -mi386pe
//objcopy -O binary -j .text test.tmp test.bin