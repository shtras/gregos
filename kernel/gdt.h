#pragma once
#include "defs.h"

struct gdt_ptr_t
{
    uint16_t limit;               // The upper 16 bits of all selector limits.
    uint32_t base;                // The address of the first gdt_entry_t struct.
}
__attribute__((packed));

struct gdt_entry_t
{
    uint16_t limit_low;           // The lower 16 bits of the limit.
    uint16_t base_low;            // The lower 16 bits of the base.
    uint8_t  base_middle;         // The next 8 bits of the base.
    uint8_t  access;              // Access flags, determine what ring this segment can be used in.
    uint8_t  granularity;
    uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));

struct idt_entry_t
{
    uint16_t base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;                 // Kernel segment selector.
    uint8_t  always0;             // This must always be zero.
    uint8_t  flags;               // More flags. See documentation.
    uint16_t base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_t
{
    uint16_t limit;
    uint32_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

struct registers_t
{
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};

void init_descriptor_tables();
