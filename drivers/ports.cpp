#include "ports.h"

uchar inb(uint16_t port)
{
    uint8_t res;
    asm volatile("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}

void outb(uint16_t port, uint8_t data)
{
    asm volatile("out %%al, %%dx" :: "a" (data), "d" (port));
}

ushort inw(uint16_t port)
{
    uint16_t res;
    asm volatile("in %%dx, %%ax" : "=a" (res) : "d" (port));
    return res;
}

void outw(uint16_t port, uint16_t data)
{
    asm volatile("out %%ax, %%dx" :: "a" (data), "d" (port));
}
