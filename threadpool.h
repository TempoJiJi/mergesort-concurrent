#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

/* Task for each thread */
typedef struct _task {
    void (*func)(void *);
    void *arg;
    struct _task *next, *last;
} task_t;

/* Structure of queue */
typedef struct {
    task_t *head, *tail;
    pthread_mutex_t mutex;
    uint32_t size;
} tqueue_t;

/* Structure of thread pool */
typedef struct {
    pthread_t *threads;
    uint32_t count;
    tqueue_t *queue;
} tpool_t;

int tqueue_init(tqueue_t *the_queue);
task_t *tqueue_pop(tqueue_t *the_queue);
int tqueue_push(tqueue_t *the_queue, task_t *task);
int tqueue_free(tqueue_t *the_queue);

tpool_t *tpool_init(tpool_t *the_pool, uint32_t count, void *(*func)(void *));
int tpool_free(tpool_t *the_pool);

#endif
