#include "kernel.h"
#include "screen.h"
#include "gdt.h"
#include "ports.h"
#include "isr.h"
#include "mem.h"

uint32_t tick = 0;

static void timer_callback(registers_t regs)
{
    tick++;
    ScreenManager::getInstance().putCursor(0, 50);
    kprintf("Tick %d   \n", tick);
    ScreenManager::getInstance().returnCursor();
    if (tick >= 100) {
        disableInterrupts(IRQ_TIMER);
    }
}

static void key_callback(registers_t regs)
{
    uchar c = inb(0x60) & 0x7f;
    kprintf("Key %d\n", c);
}

void init_timer(uint32_t frequency)
{
    // Firstly, register our timer callback.

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32_t divisor = 1193180 / frequency;
    divisor = 0xffff;

    // Send the command byte.
    outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
} 

extern "C" void enable();

void panic(const char* errMsg/* = NULL*/)
{
    asm volatile("cli");
    kprintf("Kernel panic!!! %s", errMsg ? errMsg : "");
    asm volatile("hlt");
    while(1);
}

static void page_fault(registers_t regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int present   = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;           // Write operation?
    int us = regs.err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    //int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    kprintf("Page fault! ( ");
    if (present) {kprintf("present ");}
    if (rw) {kprintf("read-only ");}
    if (us) {kprintf("user-mode ");}
    if (reserved) {kprintf("reserved ");}
    kprintf(") at ");
    kprintf("%x", faulting_address/16);
    kprintf("\n");
    panic("Page fault");
} 

extern "C" void kmain()
{
    ScreenManager& manager = ScreenManager::getInstance();
    manager.setColor(ScreenManager::COLOR_LIGHT_GREY, ScreenManager::COLOR_BLACK);
    manager.clearScreen();
    kprintf("Now loading " OS_NAME "...\n");
    
    kprintf("Initializing descriptor tables... ");
    init_descriptor_tables();
    kprintf("OK\n");
    
    register_interrupt_handler(IRQ1, &key_callback);
    register_interrupt_handler(14, &page_fault);
    register_interrupt_handler(IRQ0, &timer_callback);
    
    kprintf("Setting up paging... ");
    init_paging();
    kprintf("OK\n");

    kprintf("Enabling IRQs... ");
    setInterruptMask(0);
    enableInterrupts(IRQ_KEYBOARD | IRQ_TIMER);
    //init_timer(5);
    kprintf("OK\n");

    kprintf("Setting up initrd... ");
    kprintf("Not implemented yet :(\n");

    kprintf("Loading FS... ");
    kprintf("Not implemented yet :(\n");

    kprintf("Preparing shell... ");
    kprintf("Not implemented yet :(\n");

    kprintf(OS_NAME " has finished loading. Well, sort of. Have fun!\n");
    asm volatile("sti");
    while (1) {
        asm volatile("hlt");
    }
}

