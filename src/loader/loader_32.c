#include "comm/cpu_instr.h"
#include "loader.h"

static void read_disk(uint32_t sector, uint32_t count, void* buffer)
{
    // 选择主盘并设置扇区地址
    outb(0x1F6, 0xE0);
    outb(0x1F2, (uint8_t)(count >> 8));
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    // 设置扇区数量和起始扇区地址
    outb(0x1F2, (uint8_t)count);
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    // 发送读命令
    outb(0x1F7, 0x24);

    uint16_t* buf = (uint16_t*)buffer;
    while (count--)
    {
        // 等待磁盘就绪
        while ((inb(0x1F7) & 0x88) != 0x8)
        {
        }

        for (int i = 0; i < SECTOR_SIZE / sizeof(uint16_t); i++)
        {
            buf[i] = inw(0x1F0);
        }
    }
}

void load_kernel(void)
{
    read_disk(100, 500, (void*)SYS_KERNEL_LOAD_ADDR);
    for (;;)
    {
    }
}
