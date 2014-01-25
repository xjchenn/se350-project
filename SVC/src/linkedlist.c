#include "linkedlist.h"
#include "memory.h"
#include "utils.h"

int linkedlist_add_front(linkedlist_t *list, void *value) {
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

int linkedlist_add_back(linkedlist_t *list, void *value) {
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

void* linkedlist_pop_front(linkedlist_t *list) {
    node_t* firstNode;
    node_t* secondNode;
    void* nodeValue;

    if (list == NULL || list->first == NULL) {
        return NULL;
    }

    firstNode  = list->first;
    secondNode = firstNode->next;
    nodeValue  = firstNode->value;

    if (secondNode != NULL) {
        secondNode->prev = NULL;
    }
    list->first = secondNode;

    k_release_memory_block(firstNode);
    return nodeValue;
}

void* linkedlist_pop_back(linkedlist_t *list) {
    node_t* lastNode;
    node_t* secondLastNode;
    void* nodeValue;

    if (list == NULL || list->last == NULL) {
        return NULL;
    }

    lastNode = list->last;
    secondLastNode = lastNode->prev;
    nodeValue      = lastNode->value;

    if (secondLastNode != NULL) {
        secondLastNode->next = NULL;
    }
    list->last = secondLastNode;
    
    k_release_memory_block(lastNode);
    return nodeValue;
}
