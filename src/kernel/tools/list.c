#include "tools/list.h"

void list_node_init(list_node_t* node)
{
    node->prev = NULL;
    node->next = NULL;
}

list_node_t* list_node_prev(list_node_t* node)
{
    return node ? node->prev : NULL;
}

list_node_t* list_node_next(list_node_t* node)
{
    return node ? node->next : NULL;
}

void list_init(list_t* list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

int list_size(list_t* list)
{
    return list ? list->size : 0;
}

int list_is_empty(list_t* list)
{
    return list_size(list) == 0;
}

list_node_t* list_first(list_t* list)
{
    return list ? list->head : NULL;
}

list_node_t* list_last(list_t* list)
{
    return list ? list->tail : NULL;
}

void list_insert_after(list_t* list, list_node_t* node, list_node_t* new_node)
{
    if (!list || !new_node)
        return;
    if (!node)
    {
        list_insert_head(list, new_node);
        return;
    }
    new_node->prev = node;
    new_node->next = node->next;
    if (node->next)
        node->next->prev = new_node;
    node->next = new_node;
    if (list->tail == node)
        list->tail = new_node;
    list->size++;
}

void list_insert_before(list_t* list, list_node_t* node, list_node_t* new_node)
{
    if (!list || !new_node)
        return;
    if (!node)
    {
        list_insert_tail(list, new_node);
        return;
    }
    new_node->prev = node->prev;
    new_node->next = node;
    if (node->prev)
        node->prev->next = new_node;
    node->prev = new_node;
    if (list->head == node)
        list->head = new_node;
    list->size++;
}

void list_insert_head(list_t* list, list_node_t* new_node)
{
    if (!list || !new_node)
        return;
    new_node->prev = NULL;
    new_node->next = list->head;
    if (list->head)
        list->head->prev = new_node;
    list->head = new_node;
    if (!list->tail)
        list->tail = new_node;
    list->size++;
}

void list_insert_tail(list_t* list, list_node_t* new_node)
{
    if (!list || !new_node)
        return;
    new_node->prev = list->tail;
    new_node->next = NULL;
    if (list->tail)
        list->tail->next = new_node;
    list->tail = new_node;
    if (!list->head)
        list->head = new_node;
    list->size++;
}

void list_remove(list_t* list, list_node_t* node)
{
    if (!list || !node)
        return;
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    list->size--;
}

void list_clear(list_t* list)
{
    while (!list_is_empty(list))
        list_remove_first(list);
}

void list_remove_first(list_t* list)
{
    if (list && list->head)
        list_remove(list, list->head);
}

void list_remove_last(list_t* list)
{
    if (list && list->tail)
        list_remove(list, list->tail);
}