#include "core/task.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "tools/assert.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "tools/list.h"

static task_manager_t g_task_manager;

static void tss_init(task_t* task, uint32_t entry, uint32_t esp)
{
    kernel_memset(&task->tss, 0, sizeof(task->tss));
    int tss_selector = gdt_alloc_desc();
    assert(tss_selector >= 0);
    segment_desc_set(tss_selector, (uint32_t)&task->tss, sizeof(task->tss),
                     SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.eflags = EFLAGS_IF | EFLAGS_DEFAULT;
    task->tss_selector = tss_selector;
}

void task_init(task_t* task, const char* name, uint32_t entry, uint32_t esp)
{
    assert(task != NULL);
    assert(name != NULL);
    tss_init(task, entry, esp);
    list_node_init(&task->run_node);
    list_node_init(&task->all_node);
    kernel_strncpy(task->name, name, TASK_NAME_MAX_LEN - 1);
    task->name[TASK_NAME_MAX_LEN - 1] = '\0';
    list_insert_tail(&g_task_manager.task_list, &task->all_node);
    task->state = TASK_CREATED;
    task_set_ready(task);
}

void simple_switch(uint32_t** from, uint32_t* to);

void task_switch(task_t* from, task_t* to)
{
    assert(from != NULL && to != NULL);
    if (from == to)
    {
        return;
    }
    // 切换TSS选择子
    switch_to_tss(to->tss_selector);
    // simple_switch(&from->stack, to->stack);
}

void task_manager_init(void)
{
    g_task_manager.current_task = NULL;
    list_init(&g_task_manager.ready_list);
    list_init(&g_task_manager.task_list);
}

void task_first_init(void)
{
    task_init(&g_task_manager.first_task, "First Task", 0, 0);
    write_tr(g_task_manager.first_task.tss_selector);
    g_task_manager.current_task = &g_task_manager.first_task;
}

task_t* get_task_current(void)
{
    return g_task_manager.current_task;
}

task_t* get_task_first(void)
{
    return &g_task_manager.first_task;
}

void task_set_ready(task_t* task)
{
    assert(task != NULL);
    if (task->state == TASK_READY)
    {
        return;
    }
    task->state = TASK_READY;
    list_insert_tail(&g_task_manager.ready_list, &task->run_node);
}

void task_set_block(task_t* task)
{
    assert(task != NULL);
    list_remove(&g_task_manager.ready_list, &task->run_node);
}