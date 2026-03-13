#ifndef __OS_CFG_H__
#define __OS_CFG_H__

// GDT表的大小
#define GDT_TABLE_SIZE 256

// 内核代码段选择子
#define KERNEL_SELECTOR_CS 0x08

// 内核数据段选择子
#define KERNEL_SELECTOR_DS 0x10

// 内核栈的大小
#define KERNEL_STACK_SIZE (8 * 1024) // 8 KB

#endif // __OS_CFG_H__