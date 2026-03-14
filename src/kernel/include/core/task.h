#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"

typedef struct task_t
{
    tss_t tss; // 任务状态段
} task_t;

void task_init(task_t *task, uint32_t entry, uint32_t esp);

#endif // __TASK_H__