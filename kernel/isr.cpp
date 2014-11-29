#include "isr.h"
#include "ports.h"
#include "screen.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
} 

void irq_handler(registers_t regs)
{
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs.int_no >= 40)
    {
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);

    //kprintf("IRQ %d\n", regs.int_no);

    if (interrupt_handlers[regs.int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}

extern "C" void isr_handler(registers_t regs)
{
    if (interrupt_handlers[regs.int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}

void enableInterrupts(uint16_t mask)
{
    uint16_t currMask = getInterruptMask();
    currMask |= mask;
    setInterruptMask(currMask);
}

void disableInterrupts(uint16_t mask)
{
    uint16_t currMask = getInterruptMask();
    currMask &= ~mask;
    setInterruptMask(currMask);
}

void setInterruptMask(uint16_t mask)
{
    mask = ~mask;
    outb(0x21, mask & 0xff);
    outb(0xa1, mask >> 8);
}

uint16_t getInterruptMask()
{
    uint16_t mask = inb(0xa1);
    mask <<= 8;
    mask |= inb(0x21);
    return ~mask;
}
