#ifndef TPOOL_H
#define TPOOL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#endif

typedef struct task_t;
typedef struct calthread_t;
typedef struct thread_pool_t;
void *worker(void *arg);
void *addTask(void(*func)(void *),void *inarg);
void *cal();
int init_task_size;
int init_thread_size;
thread_pool_t *pool;
calthread_t calthread;