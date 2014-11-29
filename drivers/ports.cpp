#include "ports.h"

uchar inb(uint16_t port)
{
    uint8_t res;
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}

void outb(uint16_t port, uint8_t data)
{
    __asm__("out %%al, %%dx" :: "a" (data), "d" (port));
}

ushort inw(uint16_t port)
{
    uint16_t res;
    __asm__("in %%dx, %%ax" : "=a" (res) : "d" (port));
    return res;
}

void outw(uint16_t port, uint16_t data)
{
    __asm__("out %%ax, %%dx" :: "a" (data), "d" (port));
}
