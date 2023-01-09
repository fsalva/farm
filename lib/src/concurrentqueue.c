
#include "../include/queue.h"
#include "../include/concurrentqueue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void initQ(concurrentQ * cq, int qlen) {

    queue_init(cq->q, qlen);
    cq->capacity =  qlen;
    cq->size = 0;
    pthread_mutex_init(&(cq->mutex) , NULL);

}

int enQ(concurrentQ cq, char * f) {
    
    int retval;

    if(cq.size >= cq.capacity) return -1; 

    pthread_mutex_lock(&(cq.mutex));
    
    retval = queue_enqueue((cq.q), f);

    if(retval >= 0) cq.size++;

    pthread_mutex_unlock(&(cq.mutex));
    
    return retval;
} 

char * deQ(concurrentQ cq) {
    
    char * retval;

    if(cq.size == 0) return NULL;

    pthread_mutex_lock(&(cq.mutex));
    
    retval = queue_dequeue((cq.q));

    if(retval != NULL) cq.size--;

    pthread_mutex_unlock(&(cq.mutex));
    
    return retval;
}

int isEmpty(concurrentQ cq) { 

    int retval;

    pthread_mutex_lock(&(cq.mutex));
    
    retval = isEmptyQ(cq.q);

    pthread_mutex_unlock(&(cq.mutex));

    return retval;

}

