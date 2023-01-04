
#include "../include/queue.h"
#include "../include/files.h"
#include "../include/concurrentqueue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


int enQ(concurrentQ cq, diskfile f) {
    
    int retval;

    if(cq.size >= cq.capacity) return -1; 

    pthread_mutex_lock(&(cq.mutex));
    
    retval = enqueue(&(cq.q), f.path);

    if(retval >= 0) cq.size++;

    pthread_mutex_unlock(&(cq.mutex));
    
    return retval;
} 

diskfile * deQ(concurrentQ cq) {
    
    diskfile * retval;

    if(cq.size == 0) return NULL;

    pthread_mutex_lock(&(cq.mutex));
    
    retval = dequeue(&(cq.q));

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

