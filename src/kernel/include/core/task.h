#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

typedef struct task_t
{
    // uint32_t* stack;  // 任务栈顶指针
    tss_t tss;        // 任务状态段
    int tss_selector; // TSS选择子
} task_t;

typedef struct task_manager_t
{
    task_t* current_task; // 当前任务
    list_t ready_list;    // 就绪队列
    list_t task_list;     // 所有任务列表
    task_t first_task;    // 第一个任务（内核任务）
} task_manager_t;

void task_init(task_t* task, uint32_t entry, uint32_t esp);

void task_switch(task_t* from, task_t* to);

void task_manager_init(void);

void task_first_init(void);

task_t* get_task_current(void);

task_t* get_task_first(void);

#endif // __TASK_H__