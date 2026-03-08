__asm__(".code16gcc");

#include "loader.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"

static boot_info_t boot_info;

// 显示字符串
static void show_message(char* message)
{
    while (*message != '\0')
    {
        __asm__ volatile(
            "mov $0xe, %%ah\n\t"
            "mov %[ch], %%al\n\t"
            "int $0x10" ::[ch] "r"(*message));
        message++;
    }
}

// 内存检测
static void detect_memory(void)
{
    show_message("Memory detection...\r\n");
    SMAP_entry_t smap_entry;
    boot_info.ram_region_count = 0;
    uint32_t contID = 0;
    uint32_t signature = 0;
    uint32_t bytes = 0;
    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++)
    {
        int is_last_entry = 0;
        SMAP_entry_t* entry = &smap_entry;
        __asm__ volatile(
            "int $0x15" // INT 0x15，调用BIOS中断服务
            : "=a"(signature), "=c"(bytes), "=b"(contID)
            : "d"(0x534d4150), "D"(entry), "a"(0xe820), "b"(contID), "c"(24));
        if (signature != 0x534d4150) // "SMAP"的ASCII码
        {
            show_message("Failed to get memory map entry.\r\n");
            break;
        }
        if (bytes > 20 && (entry->ACPI & 0x0001) == 0) // 确保返回的内存地图条目大小正确，并且是ACPI兼容的
        {
            show_message("Invalid memory map entry size.\r\n");
            continue;
        }
        if (entry->Type == 1) // 可用内存
        {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
            show_message("Found usable memory region.\r\n");
        }
        if (contID == 0)
        {
            break; // 没有更多的内存区域了
        }
    }
    show_message("Memory detection completed.\r\n");
}

uint16_t gdt_table[][4] = {
    {0x0000, 0x0000, 0x00, 0x00}, // NULL段
    {0x0000, 0xffff, 0x9a, 0xcf}, // 代码段
    {0x0000, 0xffff, 0x92, 0xcf}  // 数据段
};

// 进入保护模式
static void entry_protected_mode(void)
{
    show_message("Entering protected mode...\r\n");
    cli();

    uint8_t v = inb(0x92);
    outb(0x92, v | 0x02);
    igdt((uint32_t)gdt_table, sizeof(gdt_table));
}

void loader_entry(void)
{
    show_message("Hello, World from 16-bit mode!\r\n");
    detect_memory();
    entry_protected_mode();
    for (;;)
    {
    }
}
