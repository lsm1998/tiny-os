#include "init.h"
#include "comm/cpu_instr.h"
#include "tools/assert.h"
#include "os_cfg.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "core/task.h"

void kernel_init(boot_info_t* boot_info)
{
    assert(boot_info->ram_region_count > 0);
    cpu_init();
    log_init();
    irq_init();
    time_init();
    task_manager_init();
}

static task_t init_task;
static uint32_t init_task_stack[1024];

void init_task_entry(void)
{
    int count = 0;
    for (;;)
    {
        log_printf("task is running. Count: %d", count++);
        sys_sleep(2000);
    }
}

void init_main(void)
{
    log_printf("Kernel initialized.");
    log_printf("%s Version: %s", OS_NAME, OS_VERSION);

    task_init(&init_task, "Init Task", (uint32_t)init_task_entry, (uint32_t)(init_task_stack + 1024));
    task_first_init();

    irq_enable_global();

    int count = 0;
    for (;;)
    {
        // 内核主循环
        log_printf("Kernel is running. Count: %d", count++);
        sys_sleep(1000);
    }
}
