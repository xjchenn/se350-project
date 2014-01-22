#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include "utils.h"
#include "memory.h"

typedef struct node {
    struct node *next;
    struct node *prev;
    void *value;
} node_t;

typedef struct {
    node_t *first;
    node_t *last;
    int length;
} linkedlist_t;

int linkedlist_add_front(linkedlist_t *list, void *value);
int linkedlist_add_back(linkedlist_t *list, void *value);

node_t* linkedlist_pop_front(linkedlist_t *list);
node_t* linkedlist_pop_back(linkedlist_t *list);

#endif
