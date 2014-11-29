#pragma once
#include "gdt.h"

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

enum IRQ_MASK {
    IRQ_TIMER = 1 << 0,
    IRQ_KEYBOARD = 1 << 1,
    IRQ_CASCADE = 1 << 2,
    IRQ_COM2 = 1 << 3,
    IRQ_COM1 = 1 << 4,
    IRQ_LPT2 = 1 << 5,
    IRQ_FLOPPY = 1 << 6,
    IRQ_LPT1 = 1 << 7,
    IRQ_CMOS = 1 << 8,
    MIRQ9 = 1 << 9,
    MIRQ10 = 1 << 10,
    MIRQ11 = 1 << 11,
    IRQ_MOUSE = 1 << 12,
    MIRQ13 = 1 << 13,
    MIRQ14 = 1 << 14,
    MIRQ15 = 1 << 15
};

void enableInterrupts(uint16_t mask);
void disableInterrupts(uint16_t mask);
void setInterruptMask(uint16_t mask);
uint16_t getInterruptMask();

typedef void (*isr_t)(registers_t);

extern "C" void irq_handler(registers_t regs);
void register_interrupt_handler(uint8_t n, isr_t handler); 
