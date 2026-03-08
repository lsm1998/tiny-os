#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGION_MAX 10

typedef struct boot_info_t
{
    struct
    {
        uint32_t start; // 内存区域的起始地址
        uint32_t size;  // 内存区域的大小
    } ram_region_cfg[BOOT_RAM_REGION_MAX];
    int ram_region_count;
} boot_info_t;

#endif // BOOT_INFO_H