#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

static gate_desc_t idt_table[IDT_TABLE_SIZE];

void exception_handler_unknown(void);

static void do_handler_default(exception_frame_t* frame, const char* msg)
{
    for (;;)
    {
    }
}

void do_handler_unknown(exception_frame_t* frame)
{
    do_handler_default(frame, "unknown exception");
}

void do_handler_divide_error(exception_frame_t* frame)
{
    do_handler_default(frame, "divide exception");
}

void irq_init(void)
{
    for (int i = 0; i < IDT_TABLE_SIZE; i++)
    {
        gate_desc_set(&idt_table[i], (uint32_t)exception_handler_unknown, KERNEL_SELECTOR_CS,
                      GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    }

    irq_install(IRQ0_DE, do_handler_divide_error);

    lidt((uint32_t)idt_table, sizeof(idt_table));
}

int irq_install(int irq_num, irq_handler_t handler)
{
    if(irq_num < 0 || irq_num >= IDT_TABLE_SIZE)
    {
        return -1;
    }
    gate_desc_set(&idt_table[irq_num], (uint32_t)handler, KERNEL_SELECTOR_CS,
                  GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    return 0;
}