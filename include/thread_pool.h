#ifndef TPOOL_H
#define TPOOL_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct task_t task_t;
typedef struct thread_t thread_t;
typedef struct thread_pool_t{
    pthread_mutex_t lock;
    task_t *queue;
    thread_t *t_tail,*dummy_head; //set a dummy_head to avoid head pointer point to a wrong node after longtime execution
    int tail,head,task_count,task_size;
    int shutdown; //1 for destroy
    int thread_size;
    pthread_cond_t all_done;
}thread_pool_t;

bool addTask(void(*func)(void *),void *inarg);
thread_pool_t *pool_init();
void traversal_list(thread_pool_t *pool);
void pool_destroy(thread_pool_t *pool);

#endif