#include "init.h"
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
    ASSERT(boot_info->ram_region_count > 0);
    cpu_init();
    log_init();
    memory_init(boot_info);
    irq_init();
    time_init();
    task_manager_init();
}

void move_to_first_task()
{
    task_t* current = get_task_current();
    ASSERT(current != NULL);

    tss_t* tss = &current->tss;
    __asm__ volatile("push %[ss]\n\t"
                     "push %[esp]\n\t"
                     "push %[eflags]\n\t"
                     "push %[cs]\n\t"
                     "push %[eip]\n\t"
                     "iret" ::[ss] "r"(tss->ss),
                     [esp] "r"(tss->esp),
                     [eflags] "r"(tss->eflags),
                     [cs] "r"(tss->cs),
                     [eip] "r"(tss->eip));
}

void init_main(void)
{
    log_printf("Kernel initialized.");
    log_printf("%s Version: %s", OS_NAME, OS_VERSION);

    task_first_init();

    move_to_first_task();
}
