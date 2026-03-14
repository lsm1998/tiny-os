#include "init.h"
#include "os_cfg.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/assert.h"
#include "tools/log.h"
#include "core/task.h"

void kernel_init(boot_info_t* boot_info)
{
    assert(boot_info->ram_region_count > 0);
    cpu_init();
    log_init();
    irq_init();
    time_init();
}

static task_t init_task;
static task_t first_task;
static uint32_t init_task_stack[1024];

void init_task_entry(void)
{
    int count = 0;
    for (;;)
    {
        log_printf("task is running. Count: %d", count++);
    }
}

void init_main(void)
{
    log_printf("Kernel initialized.");
    log_printf("%s Version: %s", OS_NAME, OS_VERSION);

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1023]);
    task_init(&first_task, 0, 0);

    int count = 0;
    for (;;)
    {
        // 内核主循环
        log_printf("Kernel is running. Count: %d", count++);
    }
}