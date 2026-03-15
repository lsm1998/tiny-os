#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

// 全局描述符表
static segment_desc_t gdt_table[GDT_TABLE_SIZE];

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    segment_desc_t* desc = gdt_table + (selector >> 3);

    if (limit > 0xFFFFF)
    {
        // 如果limit超过20位，设置G位，并将limit调整为以4KB为单位
        attr |= 0x8000;
        limit >>= 12;
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | ((limit >> 16) & 0x0F) << 8;
    desc->base31_24 = (base >> 24) & 0xFF;
}

void gate_desc_set(gate_desc_t* desc, uint32_t offset, uint16_t selector, uint16_t attr)
{
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
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

    // 代码段
    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D | SEG_G);
    // 数据段
    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D | SEG_G);

    // 加载GDT表
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

int gdt_alloc_desc()
{
    for (int i = 1; i < GDT_TABLE_SIZE; i++)
    {
        if (gdt_table[i].attr == 0)
        {
            return i << 3; // 返回段选择子
        }
    }
    return -1; // 没有可用的描述符
}

void switch_to_tss(int tss_selector)
{
    far_jump(tss_selector,  0);
}