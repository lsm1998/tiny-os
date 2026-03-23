#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"
#include "os_cfg.h"

// 系统进程
#define TASK_FLAG_SYSTEM (1 << 0)
// 用户进程
#define TASK_FLAG_USER 0

typedef enum task_state_t
{
    TASK_CREATED, // 创建状态
    TASK_RUNNING, // 运行状态
    TASK_SLEEP,   // 睡眠状态
    TASK_READY,   // 就绪状态
    TASK_WAITING, // 等待状态
} task_state_t;

typedef struct task_t
{
    list_node_t run_node;         // 运行队列节点
    list_node_t all_node;         // 所有进程列表节点
    list_node_t wait_node;        // 等待队列节点
    tss_t tss;                    // 进程状态段
    int tss_selector;             // TSS选择子
    task_state_t state;           // 进程状态
    char name[TASK_NAME_MAX_LEN]; // 进程名称
    int slice_ticks;              // 时间片剩余ticks数
    int time_ticks;               // 进程已运行ticks数
    int sleep_ticks;              // 进程睡眠ticks数
} task_t;

typedef struct task_manager_t
{
    task_t* current_task;  // 当前进程
    list_t ready_list;     // 就绪队列
    list_t task_list;      // 所有进程列表
    list_t sleep_list;     // 睡眠队列
    task_t first_task;     // 第一个进程（内核进程）
    task_t idle_task;      // 空闲进程
    int app_code_selector; // 应用程序代码段选择子
    int app_data_selector; // 应用程序数据段选择子
} task_manager_t;

void task_init(task_t* task, const char* name, int flags, uint32_t entry, uint32_t esp);

void task_switch(task_t* from, task_t* to);

void task_manager_init(void);

void task_first_init(void);

task_t* get_task_current(void);

task_t* get_task_first(void);

void task_set_ready(task_t* task);

void task_set_block(task_t* task);

void sys_sched_yield(void);

void sys_sleep(uint32_t ms);

task_t* task_next_run(void);

void task_dispatch(void);

void task_time_tick(void);

void task_set_sleep(task_t* task, uint32_t ticks);

void task_set_wakeup(task_t* task);

#endif // __TASK_H__
