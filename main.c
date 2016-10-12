#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "threadpool.h"
#include "list.h"

#define USAGE "usage: ./sort [thread_count] [input_count]\n"

#if defined(BENCH)
struct timespec start, end;
double cpu_time;
#endif

pthread_mutex_t lock;
static list_t *tmp_list;
static list_t *pHead = NULL;
static tpool_t *pool = NULL;

static int thread_count = 0, data_count = 0, max_cut = 0, cut_count = 0;
static int shutdown = 0;

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

list_t *merge_list(list_t *a, list_t *b)
{
    list_t *head = malloc(sizeof(list_t));
    list_t *cur = head;
    while (a && b) {
        if(a->data <= b->data) {
            cur->next = a;
            a = a->next;
        } else if(a->data > b->data) {
            cur->next = b;
            b = b->next;
        }
        cur = cur->next;
    }
    cur->next = (!a) ? b : a;
    return head->next;
}

list_t *merge_sort(list_t *list)
{
    if (!list->next || !list)
        return list;
    list_t *mid = getMiddle(list);
    list_t *half = mid->next;
    mid->next = NULL;

    return merge_list(merge_sort(list), merge_sort(half));
}

void merge(void *data)
{
    list_t *list = (list_t *) data;
    if (count_size(list) < data_count) {
        pthread_mutex_lock(&(lock));
        list_t *half = tmp_list;
        if (!half) {
            tmp_list = list;
            pthread_mutex_unlock(&(lock));
        } else {
            tmp_list = NULL;
            pthread_mutex_unlock(&(lock));
            task_t *task = (task_t *) malloc(sizeof(task_t));
            task->func = merge;
            task->arg = merge_list(list, half);
            tqueue_push(pool->queue, task);
        }
    } else {
        pHead = list;
        shutdown = 1;
#if defined(BENCH)
        clock_gettime(CLOCK_REALTIME, &end);
        cpu_time = diff_in_second(start,end);
#elif defined(CHECK)
        FILE *fp;
        fp = fopen("output","w+");
        while(pHead) {
            fprintf(fp,"%ld\n",pHead->data);
            pHead = pHead->next;
        }
        fclose(fp);
#else
        list_print(pHead);
#endif
    }
}

void cut_func(void *data)
{
    list_t *list = (list_t *) data;
    if (list && list->next && cut_count < max_cut) {
        cut_count++;

        /* cut list */
        list_t *mid = getMiddle(list);
        list_t *half = mid->next;
        mid->next = NULL;

        /* create new task: left */
        task_t *task = (task_t *) malloc(sizeof(task_t));
        task->func = cut_func;
        task->arg = list;
        tqueue_push(pool->queue, task);

        /* create new task: right */
        task = (task_t *) malloc(sizeof(task_t));
        task->func = cut_func;
        task->arg = half;
        tqueue_push(pool->queue, task);

    } else {
        merge(merge_sort(list));
    }
}

static void *task_run(void *data)
{
    (void) data;
    while (!shutdown) {
        task_t *task = tqueue_pop(pool->queue);
        if (task)
            task->func(task->arg);
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    thread_count = atoi(argv[1]);
    data_count = atoi(argv[2]);
    max_cut = (thread_count <= data_count) ? thread_count-1 : data_count-1;

    /* Read data */
    pHead = malloc(sizeof(list_t));
    pHead->next = NULL;
    list_t *e = pHead;

#if defined(BENCH) || defined(CHECK)
    FILE *fp = fopen("input","r");
    long int data;
    while ((fscanf(fp, "%ld\n", &data)) != EOF) {
        e = list_add(e, data);
    }
    fclose(fp);
#else
    printf("input unsorted data line-by-line\n");
    for (int i = 0; i < data_count; ++i) {
        long int data;
        scanf("%ld", &data);
        e = list_add(e, data);
    }
#endif

    /* initialize tasks inside thread pool */
    tmp_list = NULL;
    pthread_mutex_init(&lock,NULL);
    pool = (tpool_t *) malloc(sizeof(tpool_t));
    pool = tpool_init(pool, thread_count, task_run);

    e = pHead;

#if defined(BENCH)
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    /* launch the first task */
    task_t *_task = (task_t *) malloc(sizeof(task_t));
    _task->func = cut_func;
    _task->arg = e->next;
    tqueue_push(pool->queue, _task);

    /* release thread pool */
    tpool_free(pool);

#if defined(BENCH)
    fp = fopen("output","a+");
    if(thread_count == 1)
        fprintf(fp, "%d", data_count);
    fprintf(fp, " %lf", cpu_time);
    if(thread_count == 64)
        fprintf(fp, "\n");
    fclose(fp);
    printf("%lf\n",cpu_time);
#endif

    return 0;
}
