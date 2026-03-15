#include "core/task.h"
#include "cpu/cpu.h"
#include "tools/assert.h"
#include "tools/klib.h"
#include "os_cfg.h"

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

void task_init(task_t* task, uint32_t entry, uint32_t esp)
{
    assert(task != NULL);
    tss_init(task, entry, esp);
}

void task_switch(task_t* from, task_t* to)
{
    assert(from != NULL && to != NULL);
    if (from == to)
    {
        return;
    }
    // 切换TSS选择子
    switch_to_tss(to->tss_selector);
}