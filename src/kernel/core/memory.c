#include "core/memory.h"
#include "ipc/mutex.h"
#include "os_cfg.h"
#include "tools/bitmap.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/assert.h"
#include "cpu/mmu.h"

// 物理地址分配器
static addr_allocator_t paddr_allocator;

// 内核页目录表
static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE)));

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

pte_t* find_pte(pde_t* page_dir, uint32_t vaddr, int alloc)
{
    pte_t* page_table;

    pde_t* pde = page_dir + pde_index(vaddr);
    if (pde->present)
    {
        page_table = (pte_t*)pde_paddr(pde);
    }
    else
    {
        // 如果不存在，则考虑分配一个
        if (alloc == 0)
        {
            return (pte_t*)0;
        }

        // 分配一个物理页表
        uint32_t pg_paddr = addr_allocator_page(&paddr_allocator, 1);
        if (pg_paddr == 0)
        {
            return (pte_t*)0;
        }

        // 设置为用户可读写，将被pte中设置所覆盖
        pde->v = pg_paddr | PTE_P | PTE_W | PDE_U;

        // 为物理页表绑定虚拟地址的映射，这样下面就可以计算出虚拟地址了
        // kernel_pg_last[pde_index(vaddr)].v = pg_paddr | PTE_P | PTE_W | PTE_U;

        // 清空页表，防止出现异常
        // 这里虚拟地址和物理地址一一映射，所以直接写入
        page_table = (pte_t*)(pg_paddr);
        kernel_memset(page_table, 0, MEM_PAGE_SIZE);
    }

    return page_table + pte_index(vaddr);
}

int memory_create_map(pde_t* page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm)
{
    for (int i = 0; i < count; i++)
    {
        pte_t* pte = find_pte(page_dir, vaddr, 1);
        if (pte == NULL)
        {
            return -1;
        }

        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;

        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }

    return 0;
}

static void create_kernel_table(void)
{
    extern uint8_t s_text[], e_text[], s_data[], e_data[];
    extern uint8_t kernel_base[];

    // 地址映射表
    static memory_map_t kernel_map[] = {
        {kernel_base, s_text, 0, PTE_W},                                         // 内核栈区
        {s_text, e_text, s_text, 0},                                             // 内核代码区
        {s_data, (void*)(MEM_EBDA_START - 1), s_data, PTE_W},                    // 内核数据区
        {(void*)MEM_EXT_START, (void*)MEM_EXT_END, (void*)MEM_EXT_START, PTE_W}, // 内存分配区
    };

    // 清空页目录表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // 清空后，然后依次根据映射关系创建映射表
    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++)
    {
        memory_map_t* map = kernel_map + i;

        // 将vstart和vend对齐到页边界
        int vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE);
        int vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE);
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        // 创建映射关系
        memory_create_map(kernel_page_dir, vstart, (uint32_t)map->pstart, page_count, map->perm);
    }
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

    mem_free += bitmap_byte_count(paddr_allocator.size / MEM_PAGE_SIZE);

    ASSERT(mem_free < (uint8_t*)MEM_EBDA_START);

    // 创建内核页表并切换过去
    create_kernel_table();

    // 切换到内核页表
    mmu_set_page_dir((uint32_t)kernel_page_dir);
}

uint32_t memory_create_uvm(void)
{
    // 分配一个页目录
    pde_t* page_dir = (pde_t*)addr_allocator_page(&paddr_allocator, 1);
    if (page_dir == NULL)
    {
        return 0;
    }
    // 初始化页目录
    kernel_memset(page_dir, 0, MEM_PAGE_SIZE);

    // 将内核页目录的前面部分复制过来，保持内核地址空间一致
    uint32_t page_dir_start = pde_index(MEM_TASK_BASE);
    for (uint32_t i = 0; i < page_dir_start; i++)
    {
        page_dir[i].v = kernel_page_dir[i].v;
    }

    return (uint32_t)page_dir;
}

static int memory_alloc_page_dir(uint32_t page_dir_paddr, uint32_t vaddr, uint32_t size, uint32_t perm)
{
    uint32_t curr_vaddr = vaddr;
    int page_count = up2(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    for (int i = 0; i < page_count; i++)
    {
        uint32_t paddr = addr_allocator_page(&paddr_allocator, 1);
        if (paddr == 0)
        {
            log_printf("Failed to allocate physical page for vaddr 0x%x", curr_vaddr);
            return -1;
        }

        int ret = memory_create_map((pde_t*)page_dir_paddr, curr_vaddr, paddr, 1, perm);
        if (ret < 0)
        {
            log_printf("Failed to create page mapping for vaddr 0x%x", curr_vaddr);
            addr_free_page(&paddr_allocator, paddr, 1);
            return ret;
        }
        curr_vaddr += MEM_PAGE_SIZE;
    }
    return curr_vaddr;
}

int memory_alloc_page(uint32_t vaddr, uint32_t size, uint32_t perm)
{
    return memory_alloc_page_dir(get_task_current()->tss.cr3, vaddr, size, perm);
}

uint32_t memory_alloc_page1(void)
{
    uint32_t paddr = addr_allocator_page(&paddr_allocator, 1);
    return paddr;
}

static pde_t* current_page_dir(void)
{
    return (pde_t*)(get_task_current()->tss.cr3);
}

static uint32_t pte_paddr(pte_t* pte)
{
    return pte->phy_page_addr << 12;
}

void memory_free_page1(uint32_t paddr)
{
    if (paddr < MEM_TASK_BASE)
    {
        addr_free_page(&paddr_allocator, paddr, 1);
        return;
    }
    pte_t* pte = find_pte(current_page_dir(), paddr, 0);
    ASSERT(pte != NULL && pte->present);
    addr_free_page(&paddr_allocator, pte_paddr(pte), 1);
    // 直接清空pte，表示未映射
    pte->v = 0;
}
