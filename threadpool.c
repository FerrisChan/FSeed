#include "threadpool.h"

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp)
{
    int rc;
    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
	error_die(rc, "Pthread_create error");
}
