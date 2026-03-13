#include "cpu/cpu.h"
#include "os_cfg.h"

// 全局描述符表
static segment_desc_t gdt_table[GDT_TABLE_SIZE]; 

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    segment_desc_t* desc = gdt_table + (selector >> 3);
    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | ((limit >> 16) & 0x0F) << 8;
    desc->base31_24 = (base >> 24) & 0xFF;
}

void init_gdt(void)
{
    for (int i = 0; i < GDT_TABLE_SIZE; i++)
    {
        // 将所有段描述符初始化为无效
        segment_desc_set(i << 3, 0, 0, 0); 
    }
}

void cpu_init(void)
{
    // 初始化GDT表
    init_gdt();
    
    // 设置代码段和数据段
    segment_desc_set(1 << 3, 0x00000000, 0xFFFFF, 0x9A); // 代码段：可执行、可读、访问权限为3
    segment_desc_set(2 << 3, 0x00000000, 0xFFFFF, 0x92); // 数据段：可读写、访问权限为3

    // 加载GDT表
    uint64_t gdt_ptr = ((sizeof(gdt_table) - 1) << 16) | (uint32_t)gdt_table;
    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
}