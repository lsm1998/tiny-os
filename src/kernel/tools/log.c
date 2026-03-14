#include <stdarg.h>
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"

#define COM1_PORT 0x3F8

void log_init()
{
    // 禁用所有中断
    outb(COM1_PORT + 1, 0x00);
    // 开启 DLAB
    outb(COM1_PORT + 3, 0x80);
    // 波特率 38400: 分频值 = 3
    outb(COM1_PORT + 0, 0x03); // DLL
    outb(COM1_PORT + 1, 0x00); // DLM
    // 8位数据，无校验，1位停止位，并关闭 DLAB
    outb(COM1_PORT + 3, 0x03);
    // 开启 FIFO，清空 FIFO，14 字节阈值
    outb(COM1_PORT + 2, 0xC7);
    // IRQs enabled, RTS/DSR set
    outb(COM1_PORT + 4, 0x0B);
}

void log_printf(const char* fmt, ...)
{
    char buffer[256];

    kernel_memset(buffer, '\0', sizeof(buffer));

    va_list args;
    va_start(args, fmt);
    kernel_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char* ptr = buffer;

    while (*ptr != '\0')
    {
        // 等待发送缓冲区空
        while ((inb(COM1_PORT + 5) & 0x20) == 0);
        outb(COM1_PORT, *ptr++);
    }

    // 换行
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
}

void panic(const char* file, int line, const char* function, const char* message)
{
    log_printf("Panic at %s:%d in %s: %s", file, line, function, message);
    for (;;)
    {
        hlt();
    }
}