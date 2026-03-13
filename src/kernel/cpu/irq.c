#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

#define IDT_TABLE_SIZE 128

static gate_desc_t idt_table[IDT_TABLE_SIZE];

void exception_handler(void);

static void do_default_handler(const char* msg)
{
    for (;;)
    {
    }
}

void do_exception_handler(exception_frame_t* frame)
{
    do_default_handler("unknown exception");
}

void irq_init(void)
{
    for (int i = 0; i < IDT_TABLE_SIZE; i++)
    {
        gate_desc_set(&idt_table[i], (uint32_t)exception_handler, KERNEL_SELECTOR_CS,
                      GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    }

    lidt((uint32_t)idt_table, sizeof(idt_table));
}