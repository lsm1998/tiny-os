#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "comm/types.h"

typedef struct bitmap_t
{
    int bit_count; // 位图中位的总数
    uint8_t* bits; // 位图数据，使用 32 位整数数组存
} bitmap_t;

void bitmap_init(bitmap_t* bitmap, uint8_t* bits, int bit_count, uint8_t default_value);

int bitmap_byte_count(bitmap_t* bitmap);

int bitmap_get_bit(bitmap_t* bitmap, int index);

void bitmap_set_bit(bitmap_t* bitmap, int index, uint8_t value);

void bitmap_set_bit_range(bitmap_t* bitmap, int start_index, int count, uint8_t value);

int bitmap_count_bit(bitmap_t* bitmap, int start_index, int count, uint8_t value);

int bitmap_allocate_bits(bitmap_t* bitmap, int count, uint8_t value);

#endif // __BITMAP_H__