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

typedef struct calthread_t{
    float rate;
    pthread_t calthread;
}calthread_t;

typedef struct thread_pool_t{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    task_t *queue;
    pthread_t *threads;
    int tail,head,task_count,task_size;
    int shutdown; //1 for destroy
    int thread_size;
    
}thread_pool_t;

int init_task_size=20;
int init_thread_size=12;
thread_pool_t *pool = NULL;
calthread_t calthread;

void * worker(void* arg){
    while(1){
        pthread_mutex_lock(&pool->lock);
        while(pool->task_count==0 && !pool->shutdown){
            pthread_cond_wait(&pool->notify,&pool->lock);
        }
        if(pool->task_count==0 && pool->shutdown){
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        task_t running_task=pool->queue[pool->head];
        pool->head=(pool->head+1)%pool->task_size;
        pool->task_count--;
        pthread_mutex_unlock(&pool->lock);
        running_task.function(running_task.arg);
    }
}

void *addTask(void(*func)(void *),void *inarg){
    pthread_mutex_lock(&pool->lock);
    if(pool->task_count<pool->task_size){
        pool->queue[pool->tail].function=func;
        pool->queue[pool->tail].arg=inarg;
        pool->tail=(pool->tail+1)%pool->task_size;
        pool->task_count++;
    }
    pthread_mutex_unlock(&pool->lock);
    pthread_cond_broadcast(&pool->notify);
}

void *cal(){
    while(1){
        sleep(10);
        calthread.rate=(float)pool->task_count/(float)pool->task_size;
        if(calthread.rate>=0.8 && pool->task_size <30 &&pool->thread_size < 22){
            pthread_mutex_lock(&pool->lock);
            pool->task_size=pool->task_size+2;
            pool->queue=realloc(pool->queue,sizeof(task_t)*pool->task_size);
            int i=pool->thread_size;
            pool->thread_size=pool->thread_size+2;
            pool->threads=realloc(pool->threads,sizeof(pthread_t)*pool->thread_size);
            for(i=0;i<2;i++){
                pthread_create(&(pool->threads[i]),NULL,worker,NULL);
            }
        }
        else if(calthread.rate<=0.3 && pool->thread_size>init_thread_size && pool->task_size>init_task_size){
            pthread_mutex_lock(&pool->lock);
        }
    }
}