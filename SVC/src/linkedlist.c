#include "linkedlist.h"
#include "memory.h"
#include "utils.h"

int linkedlist_add_front(linkedlist_t *list, const void *value) {
    node_t* newNode = NULL;

    if (list == NULL) {
        return 1;
    }
    
    newNode = (node_t*) k_request_memory_block(); // what a waste of memory
    newNode->value = value;
    newNode->next  = list->first;
    newNode->prev  = NULL; 

    list->first = newNode;

    return 0;
}

int linkedlist_add_back(linkedlist_t *list, const void *value) {
    node_t* newNode = NULL;

    if (list == NULL) {
        return 1;
    }
    
    newNode = (node_t*) k_request_memory_block();
    newNode->value = value;
    newNode->next  = NULL;
    newNode->prev  = list->last;

    list->last->next = newNode;
    list->last = newNode;

    return 0;
}

node_t* linkedlist_pop_front(linkedlist_t *list) {
    node_t* firstNode;
    node_t* secondNode;

    if (list == NULL) {
        return 1;
    }

    firstNode  = list->first;
    secondNode = firstNode->next;

    list->first = secondNode;
    secondNode->prev = NULL:

    firstNode->next = NULL;
    firstNode->prev = NULL;
    return firstNode;
}

node_t* linkedlist_pop_back(linkedlist_t *list) {
    node_t* lastNode;
    node_t* secondLastNode;

    if (list == NULL) {
        return 1;
    }

    lastNode = list->last;
    secondLastNode = lastNode->prev;

    list->last = secondLastNode;
    secondLastNode->next = NULL;
    
    lastNode->next = NULL;
    lastNode->prev = NULL;
    return lastNode;
}
