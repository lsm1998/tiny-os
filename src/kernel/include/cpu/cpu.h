#ifndef __CPU_H__
#define __CPU_H__

#include "comm/types.h"

#pragma pack(1)
typedef struct segment_desc_t
{
    uint16_t limit15_0; // 段界限的低16位
    uint16_t base15_0;  // 段基址的低16位
    uint8_t base23_16;  // 段基址的中8位
    uint16_t attr;      // 段属性
    uint8_t base31_24;  // 段基址的高8位
} segment_desc_t;

typedef struct gate_desc_t
{
    uint16_t offset15_0;  // 偏移地址的低16位
    uint16_t selector;    // 代码段选择子
    uint16_t attr;        // 门描述符属性
    uint16_t offset31_16; // 偏移地址的高16位
} gate_desc_t;
#pragma pack()

typedef struct tss_t
{
    uint32_t pre_link; // 前一个TSS的链接
    uint32_t esp0, ss0, esp1, ss1, esp2, ssp2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs; // 段寄存器
    uint32_t ldt;                    // LDT选择子
    uint16_t trap;                   // 任务门标志
    uint16_t iomap;                  // I/O位图基地址
} tss_t;

// 默认的EFLAGS值，启用中断
#define EFLAGS_DEFAULT (1 << 1)

// 中断标志位
#define EFLAGS_IF (1 << 9)

// 中断门类型
#define GATE_TYPE_INT (0xE << 8)

// P位，表示门存在
#define GATE_P_PRESENT (1 << 15)

// DPL位，表示特权级0
#define GATE_DPL0 (0 << 13)

// DPL位，表示特权级3
#define GATE_DPL3 (3 << 13)

// G位，表示段界限以4KB为单位
#define SEG_G (1 << 15)

// D位，表示32位段
#define SEG_D (1 << 14)

// P位，表示段存在
#define SEG_P_PRESENT (1 << 7)

// 特权级0
#define SEG_DPL0 (0 << 5)

// 特权级3
#define SEG_DPL3 (3 << 5)

// S=0，系统段
#define SEG_S_SYSTEM 0

// S=1，代码/数据段
#define SEG_S_NORMAL (1 << 4)

// 代码段
#define SEG_TYPE_CODE (1 << 3)

// 数据段
#define SEG_TYPE_DATA (0 << 3)

// 可执行/可读
#define SEG_TYPE_TSS (9 << 3)

// 可读写
#define SEG_TYPE_RW (1 << 1)

void cpu_init(void);

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

void gate_desc_set(gate_desc_t* desc, uint32_t offset, uint16_t selector, uint16_t attr);

int gdt_alloc_desc();

void switch_to_tss(int tss_selector);

#endif // __CPU_H__