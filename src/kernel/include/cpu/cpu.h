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
#pragma pack()

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

// 可读写
#define SEG_TYPE_RW (1 << 1)

void cpu_init(void);

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

#endif // __CPU_H__