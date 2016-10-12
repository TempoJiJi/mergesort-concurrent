#include <stdlib.h>
#include <stdio.h>

#include "list.h"

/*
 * list_add inserts a new node with the given value val in the list
 * (if the value was absent) or does nothing (if the value is already present).
 */
list_t *list_add(list_t *e, val_t val)
{
    e->next = malloc(sizeof(list_t));
    e = e->next;
    e->data = val;
    e->next = NULL;

    return e;
}

list_t *getMiddle(list_t *list)
{
    if(!list)
        return list;
    list_t *slow = list;
    list_t *fast = list;
    while(fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}

int count_size(list_t *list)
{
    int count = 0;
    while(list) {
        count++;
        list = list->next;
    }
    return count;
}

void list_print(list_t *list)
{
    while (list) {
        printf("[%ld] ", list->data);
        list = list->next;
    }
    printf("\n");
}
