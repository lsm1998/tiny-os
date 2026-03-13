#ifndef __IRQ_H__
#define __IRQ_H__

#include "comm/types.h"

typedef struct exception_frame_t
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags;
} exception_frame_t;

void irq_init(void);

void do_exception_handler(exception_frame_t* frame);

#endif // __IRQ_H__