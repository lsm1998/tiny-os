#ifndef __SYSCALL_H_
#define __SYSCALL_H_

#include "comm/types.h"

// 系统调用参数个数
#define SYSCALL_PARAM_COUNT 5

typedef struct syscall_frame_t
{
    int eflags;
    int gs, fs, es, ds;
    uint32_t edi, esi, ebp, dummy, ebx, edx, ecx, eax;
    int eip, cs;
    int func_id, arg0, arg1, arg2, arg3;
    int esp, ss;
} syscall_frame_t;

void exception_handler_syscall(void);

#endif // __SYSCALL_H_