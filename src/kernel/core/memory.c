#include "core/memory.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/assert.h"

static addr_allocator_t paddr_allocator;

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

static void show_memory_info(boot_info_t* boot_info)
{
    log_printf("Memory Information:");
    for (uint32_t i = 0; i < boot_info->ram_region_count; i++)
    {
        log_printf("  Region %d: Start: 0x%x, Size: %d bytes", i, boot_info->ram_region_cfg[i].start, boot_info->ram_region_cfg[i].size);
    }
}

static uint32_t total_memory_size(boot_info_t* boot_info)
{
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < boot_info->ram_region_count; i++)
    {
        total_size += boot_info->ram_region_cfg[i].size;
    }
    return total_size;
}

void memory_init(boot_info_t* boot_info)
{
    extern uint8_t* mem_free_start;

    show_memory_info(boot_info);
    uint32_t total_size = total_memory_size(boot_info) - MEM_EXT_START;
    total_size = down2(total_size, MEM_PAGE_SIZE);
    log_printf("Total Memory Size: %d bytes", total_size);

    uint8_t* mem_free = (uint8_t*)&mem_free_start;

    addr_allocator_init(&paddr_allocator, mem_free, MEM_EXT_START, total_size, MEM_PAGE_SIZE);

    mem_free+= bitmap_byte_count(paddr_allocator.size / MEM_PAGE_SIZE);

    assert(mem_free < (uint8_t*) MEM_EBDA_START);
}