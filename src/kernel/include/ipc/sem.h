#ifndef __SEM_H__
#define __SEM_H__

#include "comm/types.h"
#include "tools/list.h"

typedef struct sem_t
{
    uint32_t count;
    list_t wait_list;
} sem_t;

void sem_init(sem_t *sem, uint32_t count);

void sem_wait(sem_t *sem);

void sem_notify(sem_t *sem);

uint32_t sem_get_count(sem_t *sem);

#endif // __SEM_H__