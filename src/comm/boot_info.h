#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

// 定义最大内存区域数量
#define BOOT_RAM_REGION_MAX 10

// 每个扇区的字节数
#define SECTOR_SIZE 512

// 内核加载地址
#define SYS_KERNEL_LOAD_ADDR (1024 * 1024) // 1MB

typedef struct boot_info_t
{
    struct
    {
        uint32_t start; // 内存区域的起始地址
        uint32_t size;  // 内存区域的大小
    } ram_region_cfg[BOOT_RAM_REGION_MAX];
    int ram_region_count;
} boot_info_t;

extern boot_info_t boot_info;

#endif // BOOT_INFO_H