#include "ipc/mutex.h"
#include "core/task.h"
#include "cpu/irq.h"
#include "tools/list.h"

void mutex_init(mutex_t* mutex)
{
    mutex->owner = NULL;
    mutex->locked_count = 0;
    list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t* mutex)
{
    irq_state_t state = irq_enter_protection();
    task_t* current = get_task_current();
    if (mutex->owner == current)
    {
        mutex->locked_count++;
    }
    else
    {
        while (mutex->owner != NULL)
        {
            list_insert_tail(&mutex->wait_list, &current->wait_node);
            task_set_block(current);
        }
        mutex->owner = current;
        mutex->locked_count = 1;
    }
    irq_exit_protection(state);
}

void mutex_unlock(mutex_t* mutex)
{
    irq_state_t state = irq_enter_protection();
    task_t* current = get_task_current();
    if (mutex->owner == current)
    {
        mutex->locked_count--;
        if (mutex->locked_count == 0)
        {
            mutex->owner = NULL;
        }
    }
    irq_exit_protection(state);
}