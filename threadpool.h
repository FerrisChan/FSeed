#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct {
    int connfd;     // 该线程的connfd
    int count;      // 线程的序号
    int status;     // 线程的状态
} threadpool;

// TODO: 实现线程池

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp);

#endif
