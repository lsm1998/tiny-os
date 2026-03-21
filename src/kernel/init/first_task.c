#include "core/task.h"
#include "tools/log.h"

int first_task_main(void)
{
    log_printf("First task initialized.");
    for (;;)
    {
        log_printf("First task is running.");
        sys_sleep(1000);
    }
    return 0;
}