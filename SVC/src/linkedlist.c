#include "linkedlist.h"
#include "k_memory.h"
#include "utils.h"

int linkedlist_init(linkedlist_t* list) {
    if (list == NULL) {
        return 1;
    }

    list->first = NULL;
    list->last = NULL;
    list->length = 0;

    return 0;
}

void* linkedlist_remove(linkedlist_t* list, void* value) {
    node_t* itr;
    void* ret;

    if (list == NULL) {
        return NULL;
    }

    if(list->first != NULL && list->first->value == value) {
        return linkedlist_pop_front(list);
    }

    if(list->last != NULL && list->last->value == value) {
        return linkedlist_pop_back(list);
    }

    itr = list->first;

    while(itr != NULL && itr->next != NULL) {
        if(itr->value == value) {
            ret = itr->value;
            if(itr->next != NULL) {
                itr->next->prev = itr->prev;
            }

            if (itr->prev != NULL) {
                itr->prev->next = itr->next;
            }

            k_release_memory_block((void*)itr);
            list->length--;
            return ret;
        }

        itr = itr->next;
    }

    return NULL;
}

int linkedlist_push_front(linkedlist_t* list, void* value) {
    node_t* newNode = NULL;

    if (list == NULL) {
        return 1;
    }

    newNode = (node_t*) k_request_memory_block(); // what a waste of memory
    newNode->next  = list->first;
    newNode->prev  = NULL;
    newNode->value = value;

    if (list->first != NULL) {
        list->first->prev = newNode;
    }

    list->first = newNode;
    if (list->last == NULL) {
        list->last = newNode;
    }

    list->length++;

    return 0;
}

int linkedlist_push_back(linkedlist_t* list, void* value) {
    node_t* newNode = NULL;

    if (list == NULL) {
        return 1;
    }

    newNode = (node_t*) k_request_memory_block();
    newNode->next  = NULL;
    newNode->prev  = list->last;
    newNode->value = value;
    if (list->last != NULL) {
        list->last->next = newNode;
    }

    list->last = newNode;
    if (list->first == NULL) {
        list->first = newNode;
    }

    list->length++;

    return 0;
}

void* linkedlist_pop_front(linkedlist_t* list) {
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

    list->length--;

    return nodeValue;
}

void* linkedlist_pop_back(linkedlist_t* list) {
    node_t* lastNode;
    node_t* secondLastNode;
    void* nodeValue;

    if (list == NULL || list->last == NULL) {
        return NULL;
    }

    lastNode       = list->last;
    secondLastNode = lastNode->prev;
    nodeValue      = lastNode->value;

    if (secondLastNode != NULL) {
        secondLastNode->next = NULL;
    }
    list->last = secondLastNode;

    k_release_memory_block(lastNode);

    list->length--;

    return nodeValue;
}
