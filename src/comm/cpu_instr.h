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

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void lgdt(uint32_t base, uint16_t size)
{
    struct
    {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) gdt = {
        .limit = size - 1,
        .base = base};

    __asm__ volatile("lgdt %0" : : "m"(gdt));
}

static inline void lidt(uint32_t base, uint16_t size)
{
    struct
    {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idt = {
        .limit = size - 1,
        .base = base};

    __asm__ volatile("lidt %0" : : "m"(idt));
}

static inline uint32_t read_cr0(void)
{
    uint32_t value;
    __asm__ volatile("mov %%cr0, %[v]" : [v] "=r"(value));
    return value;
}

static inline void write_cr0(uint32_t value)
{
    __asm__ volatile("mov %[v], %%cr0" : : [v] "r"(value));
}

static inline void far_jump(uint16_t selector, uint32_t offset)
{
    uint32_t addr[] = {offset, selector};
    __asm__ volatile("ljmpl *(%[a])" ::[a] "r"(addr));
}

#endif // __CPU_INSTR_H
