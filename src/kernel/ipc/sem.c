#include "ipc/sem.h"
#include "core/task.h"
#include "cpu/irq.h"
#include "tools/list.h"

void sem_init(sem_t* sem, uint32_t count)
{
    sem->count = count;
    list_init(&sem->wait_list);
}

void sem_wait(sem_t* sem)
{
    irq_state_t state = irq_enter_protection();
    if (sem->count > 0)
    {
        sem->count--;
    }
    else
    {
        task_t* current_task = get_task_current();
        task_set_block(current_task);
        list_insert_tail(&sem->wait_list, &current_task->wait_node);
        task_dispatch();
    }
    irq_exit_protection(state);
}

void sem_notify(sem_t* sem)
{
    irq_state_t state = irq_enter_protection();
    if (!list_is_empty(&sem->wait_list))
    {
        list_node_t* node = list_remove_first(&sem->wait_list);
        task_t* task = list_node_parent(node, task_t, wait_node);
        task_set_ready(task);
        task_dispatch();
    }
    else
    {
        sem->count++;
    }
    irq_exit_protection(state);
}

uint32_t sem_get_count(sem_t* sem)
{
    irq_state_t state = irq_enter_protection();
    uint32_t count = sem->count;
    irq_exit_protection(state);
    return count;
}