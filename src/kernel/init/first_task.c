#include "applib/lib_syscall.h"

int first_task_main(void)
{
    // log_printf("First task initialized.");
    // int i = 0;
    for (;;)
    {
        // log_printf("First task is running. Count: %d", ++i);
        // sys_sleep(1000);
        msleep(1000);
    }
    return 0;
}