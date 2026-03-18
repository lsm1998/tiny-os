#ifndef __LIST_H__
#define __LIST_H__

#include "comm/types.h"

typedef struct list_node_t
{
    struct list_node_t* prev;
    struct list_node_t* next;
} list_node_t;

typedef struct list_t
{
    list_node_t* head;
    list_node_t* tail;
    int size;
} list_t;

// 计算父结构体中成员的偏移地址
#define offset_in_parent(parent_type, node_name) ((uint32_t)(&((parent_type*)0)->node_name))

// 根据节点地址获取父结构体地址
#define parent_addr(node_addr, parent_type, node_name) ((parent_type*)((uint32_t)(node_addr) - offset_in_parent(parent_type, node_name)))

// 获取父结构体地址的简化宏
#define list_node_parent(node, parent_type, node_name) parent_addr(node, parent_type, node_name)

void list_node_init(list_node_t* node);

list_node_t* list_node_prev(list_node_t* node);

list_node_t* list_node_next(list_node_t* node);

void list_init(list_t* list);

int list_size(list_t* list);

int list_is_empty(list_t* list);

list_node_t* list_first(list_t* list);

list_node_t* list_last(list_t* list);

void list_insert_after(list_t* list, list_node_t* node, list_node_t* new_node);

void list_insert_before(list_t* list, list_node_t* node, list_node_t* new_node);

void list_insert_head(list_t* list, list_node_t* new_node);

void list_insert_tail(list_t* list, list_node_t* new_node);

list_node_t* list_remove(list_t* list, list_node_t* node);

void list_clear(list_t* list);

list_node_t* list_remove_first(list_t* list);

list_node_t* list_remove_last(list_t* list);

#endif // __LIST_H__