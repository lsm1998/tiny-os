#include "comm/cpu_instr.h"
#include "loader.h"

#define SYS_KERNEL_START_SECTOR 100
#define ELF32_IDENT_SIZE 16
#define ELF32_CLASS 1
#define ELF32_DATA_LSB 1
#define ELF32_MACHINE_386 3
#define ELF32_PT_LOAD 1
#define ELF32_PROGRAM_HEADER_MAX 16

typedef struct elf32_header_t
{
    uint8_t e_ident[ELF32_IDENT_SIZE];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf32_header_t;

typedef struct elf32_program_header_t
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf32_program_header_t;

static uint8_t kernel_sector_buffer[SECTOR_SIZE];
static elf32_header_t kernel_header;

// 小内存环境里只缓存 ELF 头和 program header，再按需读取各段内容。
static elf32_program_header_t kernel_program_headers[ELF32_PROGRAM_HEADER_MAX];

static void halt_forever(void)
{
    cli();
    for (;;)
    {
        hlt();
    }
}

static void copy_bytes(void* dst, const void* src, uint32_t size)
{
    uint8_t* dst_bytes = (uint8_t*)dst;
    const uint8_t* src_bytes = (const uint8_t*)src;

    while (size--)
    {
        *dst_bytes++ = *src_bytes++;
    }
}

static void zero_bytes(void* dst, uint32_t size)
{
    uint8_t* dst_bytes = (uint8_t*)dst;

    while (size--)
    {
        *dst_bytes++ = 0;
    }
}

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
            (*buf++) = inw(0x1F0);
        }
    }
}

static void read_kernel_bytes(uint32_t file_offset, uint32_t size, void* buffer)
{
    uint8_t* dst = (uint8_t*)buffer;

    while (size > 0)
    {
        // 内核在磁盘上仍然是 ELF 文件，这里按文件偏移换算回实际扇区位置
        uint32_t sector = SYS_KERNEL_START_SECTOR + (file_offset / SECTOR_SIZE);
        uint32_t sector_offset = file_offset % SECTOR_SIZE;
        uint32_t chunk_size = SECTOR_SIZE - sector_offset;

        if (chunk_size > size)
        {
            chunk_size = size;
        }

        read_disk(sector, 1, kernel_sector_buffer);
        copy_bytes(dst, kernel_sector_buffer + sector_offset, chunk_size);

        dst += chunk_size;
        file_offset += chunk_size;
        size -= chunk_size;
    }
}

static int elf32_header_is_valid(const elf32_header_t* header)
{
    return header->e_ident[0] == 0x7F &&
           header->e_ident[1] == 'E' &&
           header->e_ident[2] == 'L' &&
           header->e_ident[3] == 'F' &&
           header->e_ident[4] == ELF32_CLASS &&
           header->e_ident[5] == ELF32_DATA_LSB &&
           header->e_machine == ELF32_MACHINE_386 &&
           header->e_version == 1;
}

static void load_elf_segment(const elf32_program_header_t* program_header)
{
    uint32_t segment_addr = program_header->p_paddr ? program_header->p_paddr : program_header->p_vaddr;

    if (program_header->p_memsz < program_header->p_filesz || segment_addr == 0)
    {
        halt_forever();
    }

    // 先清零整个内存段，既能处理 .bss，也能覆盖 file size 之后的尾部空洞
    zero_bytes((void*)segment_addr, program_header->p_memsz);

    if (program_header->p_filesz > 0)
    {
        read_kernel_bytes(program_header->p_offset, program_header->p_filesz, (void*)segment_addr);
    }
}

void load_kernel(void)
{
    // 先读取 ELF 头，拿到 program header 表和最终入口地址
    read_kernel_bytes(0, sizeof(kernel_header), &kernel_header);
    if (!elf32_header_is_valid(&kernel_header) ||
        kernel_header.e_phentsize != sizeof(elf32_program_header_t) ||
        kernel_header.e_phnum == 0 ||
        kernel_header.e_phnum > ELF32_PROGRAM_HEADER_MAX)
    {
        halt_forever();
    }

    read_kernel_bytes(kernel_header.e_phoff,
                      kernel_header.e_phnum * sizeof(elf32_program_header_t),
                      kernel_program_headers);

    // loader 只关心需要映射到内存的 PT_LOAD 段
    for (uint32_t i = 0; i < kernel_header.e_phnum; i++)
    {
        if (kernel_program_headers[i].p_type == ELF32_PT_LOAD)
        {
            load_elf_segment(&kernel_program_headers[i]);
        }
    }

    if (kernel_header.e_entry == 0)
    {
        halt_forever();
    }

    // 入口仍然按旧接口接收 boot_info_t*，只是地址改为来自 ELF 的 e_entry
    ((void (*)(boot_info_t*))kernel_header.e_entry)(&boot_info);

    halt_forever();
}
