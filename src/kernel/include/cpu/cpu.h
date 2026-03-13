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

void cpu_init(void);

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

#endif // __CPU_H__