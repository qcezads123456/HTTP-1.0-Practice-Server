#ifndef TPOOL_H
#define TPOOL_H
#include <pthread.h>
#include <stdbool.h>

typedef struct task_t task_t;
typedef struct thread_t thread_t;
typedef struct thread_pool_t{
    pthread_mutex_t lock;
    task_t *queue;
    thread_t *t_head,*t_tail;
    int tail,head,task_count,task_size;
    int shutdown; //1 for destroy
    int thread_size;
    
}thread_pool_t;

bool addTask(void(*func)(void *),void *inarg);
thread_pool_t *pool_init();
void thread_kill();
#endif