#include "core/syscall.h"
#include "applib/lib_syscall.h"
#include "core/task.h"
#include "tools/log.h"

typedef int (*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t);

static const syscall_handler_t sys_table[] = {
    [SYS_SLEEP] = (syscall_handler_t)sys_sleep,
    [SYS_GETPID] = (syscall_handler_t)sys_getpid,
};

void do_handler_syscall(syscall_frame_t* frame)
{
    if (frame->func_id < 0 || frame->func_id >= sizeof(sys_table) / sizeof(sys_table[0]))
    {
        return;
    }
    syscall_handler_t handler = sys_table[frame->func_id];
    int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
    frame->eax = ret;
    log_printf("ok, ret=%d", ret);
}