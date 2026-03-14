#include "tools/klib.h"

void kernel_strcpy(char* dest, const char* src)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void kernel_strncpy(char* dest, const char* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }

    size_t i = 0;
    for (; i < n && src[i]; i++)
    {
        dest[i] = src[i];
    }
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }
}

void kernel_strcat(char* dest, const char* src)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }
    while (*dest)
    {
        dest++;
    }
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void kernel_strncat(char* dest, const char* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }
    while (*dest)
    {
        dest++;
    }
    for (size_t i = 0; i < n && *src; i++)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int kernel_strcmp(const char* dest, const char* src)
{
    if (dest == NULL || src == NULL)
    {
        return -1;
    }
    while (*dest && *src && *dest == *src)
    {
        dest++;
        src++;
    }
    return (unsigned char)*dest - (unsigned char)*src;
}

int kernel_strncmp(const char* dest, const char* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < n; i++)
    {
        unsigned char a = (unsigned char)dest[i];
        unsigned char b = (unsigned char)src[i];

        if (a != b)
        {
            return a - b;
        }

        if (a == '\0')
        {
            return 0;
        }
    }

    return 0;
}

int kernel_strlen(const char* str)
{
    if (str == NULL)
    {
        return 0;
    }
    int len = 0;
    while (*str++)
    {
        len++;
    }
    return len;
}

void kernel_memset(void* dest, uint8_t value, size_t n)
{
    if (dest == NULL)
    {
        return;
    }
    uint8_t* p = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++)
    {
        p[i] = value;
    }
}

void kernel_memcpy(void* dest, const void* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
}

int kernel_memcmp(const void* dest, const void* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return -1;
    }
    const uint8_t* d = (const uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++)
    {
        if (d[i] != s[i])
        {
            return d[i] - s[i];
        }
    }
    return 0;
}

void kernel_memmove(void* dest, const void* src, size_t n)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    if (d < s)
    {
        for (size_t i = 0; i < n; i++)
        {
            d[i] = s[i];
        }
    }
    else
    {
        for (size_t i = n; i > 0; i--)
        {
            d[i - 1] = s[i - 1];
        }
    }
}

void kernel_vsnprintf(char* buffer, size_t size, const char* fmt, va_list args)
{
    if (buffer == NULL || fmt == NULL || size == 0)
    {
        return;
    }

    size_t pos = 0;

    for (size_t i = 0; fmt[i] != '\0' && pos < size - 1; i++)
    {
        if (fmt[i] == '%')
        {
            i++;
            if (fmt[i] == 'd')
            {
                int value = va_arg(args, int);
                char num_buffer[12];
                kernel_itoa(value, num_buffer, 10);
                kernel_strncpy(buffer + pos, num_buffer, size - pos);
                pos += kernel_strlen(num_buffer);
            }
            else if (fmt[i] == 's')
            {
                const char* str = va_arg(args, const char*);
                kernel_strncpy(buffer + pos, str, size - pos);
                pos += kernel_strlen(str);
            }
            else
            {
                buffer[pos++] = fmt[i];
            }
        }
        else
        {
            buffer[pos++] = fmt[i];
        }
    }

    buffer[pos] = '\0';
}

void kernel_itoa(int value, char* str, int base)
{
    if (str == NULL || base < 2 || base > 36)
    {
        return;
    }

    char* ptr = str;
    int num = value;

    if (num < 0 && base == 10)
    {
        *ptr++ = '-';
        num = -num;
    }

    char temp[36];
    int i = 0;

    do
    {
        int rem = num % base;
        temp[i++] = (rem > 9) ? (rem - 10 + 'a') : (rem + '0');
        num /= base;
    } while (num > 0);

    for (int j = i - 1; j >= 0; j--)
    {
        *ptr++ = temp[j];
    }
    *ptr = '\0';
}

void kernel_snprintf(char* buffer, size_t size, const char* fmt, ...)
{
    if (buffer == NULL || fmt == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);
    kernel_vsnprintf(buffer, size, fmt, args);
    va_end(args);
}