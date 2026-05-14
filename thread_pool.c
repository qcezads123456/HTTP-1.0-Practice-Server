#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "thread_pool.h"

typedef enum state{
    IDLE,
    RUNNING,
    TERMINATE
}state;

typedef struct task_t
{
    void (*function)(void *arg);
    void *arg;
}task_t;

typedef struct calthread_t{
    float rate;
    pthread_t calthread;
}calthread_t;
typedef struct thread_t
{
    struct thread_t *prev, *next;
    pthread_t thread;
    pthread_cond_t notify;
    state s;
}thread_t;

typedef struct thread_pool_t{
    pthread_mutex_t lock;
    task_t *queue;
    thread_t *t_head,*t_tail;
    int tail,head,task_count,task_size;
    int shutdown; //1 for destroy
    int thread_size;
    
}thread_pool_t;

int init_task_size=20;
int init_thread_size=12;
thread_pool_t *pool = NULL;
calthread_t calthread;

void * worker(void* arg){
    thread_t *self=(thread_t*)arg;
    while(1){
        pthread_mutex_lock(&pool->lock);
        while((pool->task_count==0 || self!=pool->t_head) && !pool->shutdown && self->s != TERMINATE){
            self->s= IDLE;
            pthread_cond_wait(&(self->notify),&pool->lock);
        }
        if(pool->task_count==0 && (pool->shutdown || self->s == TERMINATE)){
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        self->s=RUNNING;
        pool->t_head=self->next;
        self->next=NULL;
        if(pool->t_head!=NULL){
            pool->t_head->prev=NULL;
        }
        else{
            pool->t_tail=NULL;
        }
        task_t running_task=pool->queue[pool->head];
        pool->head=(pool->head+1)%pool->task_size;
        pool->task_count--;
        pthread_mutex_unlock(&pool->lock);
        running_task.function(running_task.arg);
        pthread_mutex_lock(&pool->lock);
        if(pool->t_tail==NULL){
            pool->t_tail=self;
            pool->t_head=self;
            self->prev=NULL;
        }
        else{
            pool->t_tail->next=self;
            self->prev=pool->t_tail;
            self->next=NULL;
            pool->t_tail=self;
        }
        pthread_mutex_unlock(&pool->lock);
    }
    pthread_cond_destroy(&self->notify);
    free(self);
    return NULL;
}

bool addTask(void(*func)(void *),void *inarg){
    pthread_mutex_lock(&pool->lock);
    bool success=0;
    if(pool->task_count<pool->task_size){
        pool->queue[pool->tail].function=func;
        pool->queue[pool->tail].arg=inarg;
        pool->tail=(pool->tail+1)%pool->task_size;
        pool->task_count++;
        success=1;
    }
    if(pool->t_head!=NULL){
        pthread_cond_signal(&(pool->t_head->notify));
        pthread_mutex_unlock(&pool->lock);
    }
    pthread_mutex_unlock(&pool->lock);
    return success;
}
void *cal(){
    struct timespec ts;
    ts.tv_nsec=200000000;
    ts.tv_sec=0;
    while(1){
        nanosleep(&ts,NULL);
        pthread_mutex_lock(&pool->lock);
        if(!pool->shutdown){
            calthread.rate=(float)pool->task_count/(float)pool->task_size;
            if(calthread.rate>=0.8){
                task_t *newqueue,*oldqueue;
                newqueue=malloc(sizeof(task_t)*pool->task_size*2);
                oldqueue=pool->queue;
                for(int i=0;i<pool->task_count;i++){
                    newqueue[i]=pool->queue[(pool->head+i)%pool->task_size];
                }
                pool->head=0;
                pool->tail=pool->task_count;
                pool->queue=newqueue;
                pool->task_size *= 2;
                free(oldqueue);
                thread_t *newthread;
                newthread=(thread_t*)malloc(sizeof(thread_t));
                pthread_cond_init(&(newthread->notify), NULL);
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
                pthread_create(&newthread->thread,&attr,worker,newthread);
                pthread_attr_destroy(&attr);
                if(pool->t_tail==NULL){
                    pool->t_tail=newthread;
                    pool->t_head=newthread;
                    newthread->prev=NULL;
                }
                else{
                    pool->t_tail->next=newthread;
                    newthread->prev=pool->t_tail;
                    newthread->next=NULL;
                    pool->t_tail=newthread;
                }
                pool->thread_size += 1;
            }
            else if(calthread.rate<=0.3){
                if(pool->task_size>init_task_size){
                    task_t *newqueue,*oldqueue;
                    bool flag=0;
                    if(pool->task_size*0.9>=init_task_size){
                        newqueue=malloc(sizeof(task_t)*(pool->task_size*0.9));
                    }
                    else{
                        newqueue=malloc(sizeof(task_t)*(init_task_size));
                        flag=1;
                    }
                        oldqueue=pool->queue;
                    for(int i=0;i<pool->task_count;i++){
                        newqueue[i]=pool->queue[(pool->head+i)%pool->task_size];
                    }
                    pool->head=0;
                    pool->tail=pool->task_count;
                    pool->queue=newqueue;
                    if(!flag){
                        pool->task_size *= 0.9;
                    }
                    else{
                        pool->task_size=init_task_size;
                    }
                    free(oldqueue);
                }
                if(pool->thread_size>init_thread_size){
                    thread_t *old=pool->t_head;
                    if(old!=NULL){
                        pool->t_head=old->next;
                        old->next=NULL;
                        if(pool->t_head!=NULL){
                            pool->t_head->prev=NULL;
                        }
                        else{
                            pool->t_tail=NULL;
                        }
                        old->s=TERMINATE;
                        pthread_cond_signal(&old->notify);
                        pool->thread_size -= 1;
                    }
                }
            }
            pthread_mutex_unlock(&pool->lock);
        }
        else{
            pthread_mutex_unlock(&pool->lock);
            break;
        }
    }
    return NULL;
}
void thread_kill(){
    pthread_mutex_lock(&pool->lock);
    pool->shutdown=1;
    while(pool->t_head!=NULL){
        pthread_cond_signal(&pool->t_head->notify);
        pool->t_head=pool->t_head->next;
    }
    pthread_mutex_unlock(&pool->lock);
}