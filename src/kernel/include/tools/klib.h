#ifndef __KLIB_H__
#define __KLIB_H__

#include <stdarg.h>
#include "comm/types.h"

void kernel_strcpy(char* dest, const char* src);

void kernel_strncpy(char* dest, const char* src, size_t n);

void kernel_vsnprintf(char* buffer, size_t size, const char* fmt, va_list args);

void kernel_snprintf(char* buffer, size_t size, const char* fmt, ...);

void kernel_strcat(char* dest, const char* src);

void kernel_strncat(char* dest, const char* src, size_t n);

int kernel_strcmp(const char* dest, const char* src);

int kernel_strncmp(const char* dest, const char* src, size_t n);

int kernel_strlen(const char* str);

void kernel_memset(void* dest, uint8_t value, size_t n);

void kernel_memcpy(void* dest, const void* src, size_t n);

int kernel_memcmp(const void* dest, const void* src, size_t n);

void kernel_memmove(void* dest, const void* src, size_t n);

void kernel_itoa(int value, char* str, int base);

#endif // __KLIB_H__