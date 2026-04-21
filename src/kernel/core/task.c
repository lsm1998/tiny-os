#include "core/task.h"
#include "core/memory.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "tools/assert.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "tools/list.h"
#include "cpu/irq.h"
#include "cpu/mmu.h"

// 任务管理器
static task_manager_t g_task_manager;

// 空闲任务栈
static uint32_t g_idle_task_stack[IDLE_STACK_SIZE];

static int tss_init(task_t* task, int flags, uint32_t entry, uint32_t esp)
{
    kernel_memset(&task->tss, 0, sizeof(task->tss));
    int tss_selector = gdt_alloc_desc();
    ASSERT(tss_selector >= 0);
    segment_desc_set(tss_selector, (uint32_t)&task->tss, sizeof(task->tss),
                     SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);

    uint32_t kernel_stack = memory_alloc_page1();
    if (kernel_stack == 0)
    {
        goto tss_init_failed;
    }

    int code_selector = g_task_manager.app_code_selector | SEG_CPL3;
    int data_selector = g_task_manager.app_data_selector | SEG_CPL3;

    if (flags & TASK_FLAG_SYSTEM)
    {
        code_selector = KERNEL_SELECTOR_CS;
        data_selector = KERNEL_SELECTOR_DS;
    }

    task->tss.eip = entry;
    task->tss.esp = esp;
    // 栈顶地址
    task->tss.esp0 = kernel_stack + MEM_PAGE_SIZE;
    task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.ss = data_selector;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = data_selector;
    task->tss.cs = code_selector;
    task->tss.eflags = EFLAGS_IF | EFLAGS_DEFAULT;
    task->tss_selector = tss_selector;

    // 为每个任务创建独立的页目录表，保持内核地址空间一致
    uint32_t page_dir = memory_create_uvm();
    if (page_dir == 0)
    {
        goto tss_init_failed;
    }
    task->tss.cr3 = page_dir;
    return 0;

tss_init_failed:
    gdt_free_sel(tss_selector);
    if (kernel_stack > 0)
    {
        memory_free_page1(kernel_stack);
    }
    return -1;
}

void task_init(task_t* task, const char* name, int flags, uint32_t entry, uint32_t esp)
{
    ASSERT(task != NULL);
    ASSERT(name != NULL);
    tss_init(task, flags, entry, esp);
    list_node_init(&task->run_node);
    list_node_init(&task->all_node);
    list_node_init(&task->wait_node);
    kernel_strncpy(task->name, name, TASK_NAME_MAX_LEN - 1);
    task->name[TASK_NAME_MAX_LEN - 1] = '\0';
    task->state = TASK_CREATED;
    task->time_ticks = TASK_TIME_TICKS_DEFAULT;
    task->slice_ticks = task->time_ticks;
    task->sleep_ticks = 0;
    task->pid = (uint32_t)task;

    irq_state_t state = irq_enter_protection();
    task_set_ready(task);
    list_insert_tail(&g_task_manager.task_list, &task->all_node);
    irq_exit_protection(state);
}

void simple_switch(uint32_t** from, uint32_t* to);

static void idle_task_entry(void)
{
    for (;;)
    {
        hlt();
    }
}

void task_switch(task_t* from, task_t* to)
{
    ASSERT(from != NULL && to != NULL);
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
    // 初始化应用程序代码段和数据段选择子
    int selector = gdt_alloc_desc();
    ASSERT(selector >= 0);
    segment_desc_set(selector, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL3 | SEG_D | SEG_TYPE_RW | SEG_TYPE_DATA | SEG_S_NORMAL);
    g_task_manager.app_data_selector = selector;

    selector = gdt_alloc_desc();
    ASSERT(selector >= 0);
    segment_desc_set(selector, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL3 | SEG_D | SEG_TYPE_RW | SEG_TYPE_CODE | SEG_S_NORMAL);
    g_task_manager.app_code_selector = selector;

    g_task_manager.current_task = NULL;
    list_init(&g_task_manager.ready_list);
    list_init(&g_task_manager.task_list);
    list_init(&g_task_manager.sleep_list);

    // 初始化空闲任务
    task_init(&g_task_manager.idle_task,
              "Idle Task",
              TASK_FLAG_SYSTEM,
              (uint32_t)idle_task_entry,
              (uint32_t)(g_idle_task_stack + IDLE_STACK_SIZE));
}

void task_first_init(void)
{
    void first_task_entry(void);
    extern uint8_t s_first_task[], e_first_task[];

    uint32_t copy_size = (uint32_t)(e_first_task - s_first_task);
    uint32_t alloc_size = 10 * MEM_PAGE_SIZE;
    ASSERT(copy_size < alloc_size);

    uint32_t first_start = (uint32_t)first_task_entry;
    task_init(&g_task_manager.first_task, "First Task", TASK_FLAG_USER, first_start, first_start + alloc_size);
    write_tr(g_task_manager.first_task.tss_selector);
    g_task_manager.current_task = &g_task_manager.first_task;

    mmu_set_page_dir(g_task_manager.first_task.tss.cr3);

    memory_alloc_page(first_start, alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void*)first_start, s_first_task, copy_size);
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
    ASSERT(task != NULL);
    if (task == &g_task_manager.idle_task)
    {
        return;
    }
    if (task->state == TASK_READY)
    {
        return;
    }
    task->state = TASK_READY;
    list_insert_tail(&g_task_manager.ready_list, &task->run_node);
}

void task_set_block(task_t* task)
{
    if (task == NULL || task == &g_task_manager.idle_task)
    {
        return;
    }
    list_remove(&g_task_manager.ready_list, &task->run_node);
    // task->state = TASK_WAITING;
}

task_t* task_next_run(void)
{
    list_node_t* task_nodex = list_first(&g_task_manager.ready_list);
    if (task_nodex == NULL)
    {
        return &g_task_manager.idle_task;
    }
    return list_node_parent(task_nodex, task_t, run_node);
}

void task_dispatch(void)
{
    irq_state_t state = irq_enter_protection();
    task_t* to = task_next_run();
    if (to != NULL && to != g_task_manager.current_task)
    {
        task_t* from = g_task_manager.current_task;
        g_task_manager.current_task = to;
        to->state = TASK_RUNNING;
        task_switch(from, to);
    }
    irq_exit_protection(state);
}

void sys_sched_yield(void)
{
    irq_state_t state = irq_enter_protection();
    if (list_size(&g_task_manager.ready_list) > 1)
    {
        task_t* current = g_task_manager.current_task;
        task_set_block(current);
        task_set_ready(current);
        task_dispatch();
    }
    irq_exit_protection(state);
}

void task_time_tick(void)
{
    task_t* current = g_task_manager.current_task;
    if (current == NULL)
    {
        return;
    }
    if (--current->slice_ticks <= 0)
    {
        current->slice_ticks = current->time_ticks;
        task_set_block(current);
        task_set_ready(current);
        task_dispatch();
    }

    list_node_t* node = list_first(&g_task_manager.sleep_list);
    while (node != NULL)
    {
        // 先保存下一个节点，防止当前节点被唤醒后从睡眠队列中移除导致访问错误
        list_node_t* next = node->next;
        task_t* task = list_node_parent(node, task_t, run_node);
        if (--task->sleep_ticks <= 0)
        {
            task_set_wakeup(task);
            task_set_ready(task);
        }
        node = next;
    }
    task_dispatch();
}

void task_set_sleep(task_t* task, uint32_t ticks)
{
    if (task == NULL || ticks == 0)
    {
        return;
    }
    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_tail(&g_task_manager.sleep_list, &task->run_node);
}

void task_set_wakeup(task_t* task)
{
    if (task == NULL)
    {
        return;
    }
    list_remove(&g_task_manager.sleep_list, &task->run_node);
}

int sys_sleep(uint32_t ms)
{
    if (ms < OS_TICKS_MS)
    {
        ms = OS_TICKS_MS;
    }

    irq_state_t state = irq_enter_protection();
    task_t* current = g_task_manager.current_task;
    if (current != NULL)
    {
        task_set_block(current);
        // 将毫秒转换为ticks数 向上取整
        task_set_sleep(current, (ms + OS_TICKS_MS - 1) / OS_TICKS_MS);
        task_dispatch();
    }
    irq_exit_protection(state);
    return 0;
}

int sys_getpid(void)
{
    task_t* current = g_task_manager.current_task;
    if (current == NULL)
    {
        return -1;
    }
    return current->pid;
}