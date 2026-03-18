#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "core/task.h"
#include "tools/list.h"

typedef struct mutex_t
{
    task_t* owner;
    uint32_t locked_count;
    list_t wait_list;
} mutex_t;

void mutex_init(mutex_t* mutex);

void mutex_lock(mutex_t* mutex);

void mutex_unlock(mutex_t* mutex);

#endif // __MUTEX_H__