#ifndef __MMU_H__
#define __MMU_H__

#include "comm/types.h"
#include "comm/cpu_instr.h"

#define PDE_CNT 1024

typedef union pde_t
{
    uint32_t val;
    struct
    {
        uint32_t present : 1;          // 是否存在
        uint32_t rw : 1;               // 可读写
        uint32_t user : 1;             // 用户态可访问
        uint32_t write_through : 1;    // 写直达
        uint32_t cache_disable : 1;    // 禁止缓存
        uint32_t accessed : 1;         // 已访问
        uint32_t reserved : 6;         // 保留
        uint32_t page_table_base : 20; // 页表基地址
    };
} pde_t;

static inline void mmu_set_page_dir(uint32_t paddr)
{
    write_cr3(paddr);
}

#endif // __MMU_H__