#include "init.h"
#include "os_cfg.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/assert.h"

void kernel_init(boot_info_t* boot_info)
{
    assert(boot_info->ram_region_count > 0);
    cpu_init();
    log_init();
    irq_init();
    time_init();
}

void init_main(void)
{
    log_printf("Kernel initialized.");
    log_printf("Version: %s", OS_VERSION);

    int a = 3 / 0; // 故意制造一个除零错误来测试异常处理
    // irq_enable_global();
    for (;;)
    {
        // 内核主循环
    }
}