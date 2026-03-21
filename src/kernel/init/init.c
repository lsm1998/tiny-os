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
#include "core/memory.h"

void kernel_init(boot_info_t* boot_info)
{
    assert(boot_info->ram_region_count > 0);
    cpu_init();
    memory_init(boot_info);
    log_init();
    irq_init();
    time_init();
    task_manager_init();
}

void move_to_first_task()
{
    void first_task_entry();
    first_task_entry();
}

void init_main(void)
{
    log_printf("Kernel initialized.");
    log_printf("%s Version: %s", OS_NAME, OS_VERSION);

    task_first_init();

    move_to_first_task();
}
