#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"

void kernel_init(boot_info_t* boot_info)
{
    cpu_init();
}

void init_main(void)
{
    int a = 3/0; // 故意制造一个除零错误来测试异常处理
    for (;;)
    {
        // 内核主循环
    }
}