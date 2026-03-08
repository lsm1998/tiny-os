#ifndef __CPU_INSTR_H
#define __CPU_INSTR_H

#include "types.h"

static inline void cli(void)
{
    __asm__ volatile("cli");
}

static inline void sti(void)
{
    __asm__ volatile("sti");
}

static inline void hlt(void)
{
    __asm__ volatile("hlt");
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void igdt(uint32_t start, uint32_t size)
{
    struct {
        uint16_t limit;
        uint32_t start0_15;
        uint32_t start16_31;
    } gdt;
    gdt.limit = size - 1;
    gdt.start0_15 = start & 0xFFFF;
    gdt.start16_31 = (start >> 16) & 0xFFFF;
    __asm__ volatile("lgdt %0" : : "m"(gdt));
}

#endif // __CPU_INSTR_H