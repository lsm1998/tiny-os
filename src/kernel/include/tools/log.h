#ifndef __LOG_H__
#define __LOG_H__

void log_init();

void log_printf(const char *fmt, ...);

void panic(const char* file, int line, const char* function, const char* message);

#endif // __LOG_H__