#ifndef __IRQ_H__
#define __IRQ_H__

#include "comm/types.h"

#define IDT_TABLE_SIZE 128

#define IRQ0_DE 0

typedef struct exception_frame_t
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t irq_num, error_code;
    uint32_t eip, cs, eflags;
} exception_frame_t;

typedef void (*irq_handler_t)(exception_frame_t* frame);

void irq_init(void);

void do_handler_unknown(exception_frame_t* frame);

void do_handler_divide_error(exception_frame_t* frame);

int irq_install(int irq_num, irq_handler_t handler);

#endif // __IRQ_H__