#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "comm/boot_info.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"

typedef struct addr_allocator_t
{
    bitmap_t bitmap;    // 位图，跟踪每页的分配状态
    mutex_t mutex;      // 互斥锁，用于保护内存分配操作
    uint32_t start;     // 可分配地址的起始地址
    uint32_t size;      // 可分配地址的总大小
    uint32_t page_size; // 每页的大小
} addr_allocator_t;

typedef struct memory_map_t
{
    void* vstart;  // 虚拟地址开始
    void* vend;    // 虚拟地址结束
    void* pstart;  // 物理地址
    uint32_t perm; // 访问权限
} memory_map_t;

void memory_init(boot_info_t* boot_info);

uint32_t memory_create_uvm(void);

int memory_alloc_page(uint32_t vaddr, uint32_t size, uint32_t perm);

#endif // __MEMORY_H__