#include "tools/bitmap.h"
#include "tools/klib.h"

void bitmap_init(bitmap_t* bitmap, uint8_t* bits, int bit_count, uint8_t default_value)
{
    bitmap->bit_count = bit_count;
    bitmap->bits = bits;

    // 计算需要的字节数
    int byte_count = bitmap_byte_count(bitmap->bit_count);

    // 初始化位图数据
    kernel_memset(bits, default_value ? 0xFF : 0x00, byte_count);
}

int bitmap_byte_count(int bit_count)
{
    return (bit_count + 7) / 8;
}

int bitmap_get_bit(bitmap_t* bitmap, int index)
{
    if (index < 0 || index >= bitmap->bit_count)
        return -1; // 越界

    int byte_index = index / 8;
    int bit_offset = index % 8;

    return (bitmap->bits[byte_index] >> bit_offset) & 0x01;
}

void bitmap_set_bit(bitmap_t* bitmap, int index, uint8_t value)
{
    if (bitmap == NULL || index < 0 || index >= bitmap->bit_count)
        return; // 越界或无效参数

    int byte_index = index / 8;
    int bit_offset = index % 8;
    uint8_t mask = (uint8_t)(1u << bit_offset);

    if (value)
        bitmap->bits[byte_index] |= mask;
    else
        bitmap->bits[byte_index] &= (uint8_t)~mask;
}

void bitmap_set_bit_range(bitmap_t* bitmap, int start_index, int count, uint8_t value)
{
    if (bitmap == NULL || start_index < 0 || start_index >= bitmap->bit_count || count <= 0)
        return; // 越界或无效参数

    int end_index = start_index + count;
    if (end_index > bitmap->bit_count)
        end_index = bitmap->bit_count; // 限制在位图范围内

    for (int i = start_index; i < end_index; i++)
    {
        bitmap_set_bit(bitmap, i, value);
    }
}

int bitmap_count_bit(bitmap_t* bitmap, int start_index, int count, uint8_t value)
{
    if (start_index < 0 || start_index >= bitmap->bit_count || count <= 0)
        return -1; // 越界或无效参数

    int end_index = start_index + count;
    if (end_index > bitmap->bit_count)
        end_index = bitmap->bit_count; // 限制在位图范围内

    int bit_value = value ? 1 : 0;
    int bit_count = 0;

    for (int i = start_index; i < end_index; i++)
    {
        if (bitmap_get_bit(bitmap, i) == bit_value)
            bit_count++;
    }

    return bit_count;
}

int bitmap_allocate_bits(bitmap_t* bitmap, int count, uint8_t value)
{
    if (count <= 0 || count > bitmap->bit_count)
    {
        return -1; // 无效参数
    }

    int bit_value = value ? 1 : 0;

    for (int i = 0; i <= bitmap->bit_count - count; i++)
    {
        if (bitmap_count_bit(bitmap, i, count, bit_value) == count)
        {
            bitmap_set_bit_range(bitmap, i, count, !bit_value); // 分配后设置为相反的值
            return i;
        }
    }
    return -1; // 没有足够连续的位可用
}
