#include "core/memory.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"
#include "tools/log.h"

static addr_allocator_t allocator;

static void addr_allocator_init(addr_allocator_t* allocator, uint8_t* bits, uint32_t start, uint32_t size, uint32_t page_size)
{
    mutex_init(&allocator->mutex);
    allocator->start = start;
    allocator->size = size;
    allocator->page_size = page_size;
    bitmap_init(&allocator->bitmap, bits, size / page_size, 0);
}

static uint32_t addr_allocator_page(addr_allocator_t* allocator, uint32_t page_count)
{
    uint32_t addr = 0;
    mutex_lock(&allocator->mutex);
    int page_index = bitmap_allocate_bits(&allocator->bitmap, page_count, 0);
    if (page_index >= 0)
    {
        addr = allocator->start + page_index * allocator->page_size;
    }
    mutex_unlock(&allocator->mutex);
    return addr;
}

static void addr_free_page(addr_allocator_t* allocator, uint32_t addr, uint32_t page_count)
{
    if (addr < allocator->start || addr >= allocator->start + allocator->size)
        return; // 地址不在分配范围内
    uint32_t page_index = (addr - allocator->start) / allocator->page_size;
    mutex_lock(&allocator->mutex);
    // 释放后设置为0，表示空闲
    bitmap_set_bit_range(&allocator->bitmap, page_index, page_count, 0);
    mutex_unlock(&allocator->mutex);
}

void memory_init(boot_info_t* boot_info)
{
    addr_allocator_t allocator;
    uint8_t bits[8];
    addr_allocator_init(&allocator, bits, 0x1000, 64 * 4096, 4096);
    for (int i = 0; i < 32; i++)
    {
        uint32_t addr = addr_allocator_page(&allocator, 2); 
        log_printf("Allocated page at address: 0x%X", addr);
    }

    for (int i = 0; i < 32; i++)
    {
        uint32_t addr = 0x1000 + i * 2 * 4096; // 假设之前分配的地址
        addr_free_page(&allocator, addr, 2);
        log_printf("Freed page at address: 0x%X", addr);
    }
}