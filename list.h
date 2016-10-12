#ifndef LLIST_H_
#define LLIST_H_

#include <stdint.h>

typedef intptr_t val_t;

typedef struct _LIST_T {
    val_t data;
    struct _LIST_T *next;
} list_t;


list_t *list_add(list_t *e, val_t val);
list_t *getMiddle(list_t *list);
int count_size(list_t *list);
void list_print(list_t *list);

#endif
