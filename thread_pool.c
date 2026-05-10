#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "thread_pool.h"
typedef struct task_t
{
    void (*function)(void *arg);
    void *arg;
}task_t;
typedef struct thread_pool_t
{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    task_t *queue;
    pthread_t *threads;
    int tail,head,task_count,task_size;
    int shutdown; //1 for destroy
    int thread_size;
    
}thread_pool_t;

void * worker(void* arg){
    thread_pool_t *pool=(thread_pool_t*)arg;
    while(1){
        pthread_mutex_lock(&pool->lock);
        if(pool->task_count==0 && pool->shutdown){
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        while(pool->task_count==0){
            pthread_cond_wait(&pool->notify,&pool->lock);
        }
        task_t running_task=pool->queue[pool->head];
        pool->head=(pool->head+1)%pool->task_size;
        pool->task_count--;
        pthread_mutex_unlock(&pool->lock);
        running_task.function(running_task.arg);
    }
}
void *addTask();