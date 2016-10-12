#include "threadpool.h"

int tqueue_init(tqueue_t *the_queue)
{
    the_queue->head = NULL;
    the_queue->tail = NULL;
    pthread_mutex_init(&(the_queue->mutex), NULL);
    the_queue->size = 0;
    return 0;
}

task_t *tqueue_pop(tqueue_t *the_queue)
{
    task_t *ret;
    pthread_mutex_lock(&(the_queue->mutex));
    ret = the_queue->tail;
    if (ret) {
        the_queue->tail = ret->last;
        if (the_queue->tail) {
            the_queue->tail->next = NULL;
        } else {
            the_queue->head = NULL;
        }
        the_queue->size--;
    }
    pthread_mutex_unlock(&(the_queue->mutex));
    return ret;
}

int tqueue_push(tqueue_t *the_queue, task_t *task)
{
    pthread_mutex_lock(&(the_queue->mutex));
    task->last = NULL;
    task->next = the_queue->head;
    if (the_queue->head)
        the_queue->head->last = task;
    the_queue->head = task;
    if (the_queue->size++ == 0)
        the_queue->tail = task;
    pthread_mutex_unlock(&(the_queue->mutex));
    return 0;
}

int tqueue_free(tqueue_t *the_queue)
{
    task_t *cur = the_queue->head;
    while (cur) {
        the_queue->head = the_queue->head->next;
        free(cur);
        cur = the_queue->head;
    }
    pthread_mutex_destroy(&(the_queue->mutex));
    return 0;
}

tpool_t *tpool_init(tpool_t *the_pool, uint32_t tcount, void *(*func)(void *))
{
    the_pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * tcount);
    the_pool->count = tcount;
    the_pool->queue = (tqueue_t *) malloc(sizeof(tqueue_t));

    tqueue_init(the_pool->queue);
    for (uint32_t i = 0; i < tcount; ++i)
        pthread_create(&(the_pool->threads[i]), NULL, func, NULL);
    return the_pool;
}

int tpool_free(tpool_t *pool)
{
    for (uint32_t i = 0; i < pool->count; ++i)
        pthread_join(pool->threads[i], NULL);

    free(pool->threads);
    tqueue_free(pool->queue);
    return 0;
}
