#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H


#include "./queue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Rappresenta una coda concorrente.
 * 
 * Ha dimensione massima fissata (capacity), conta gli elementi al suo interno (size) e punta al prossimo elemento (next).  
 */
typedef struct {
    
    int size;
    
    int capacity;

    queue * q;

    pthread_mutex_t mutex;

} concurrentQ;

void initQ(concurrentQ * cq, int qlen);

int enQ(concurrentQ cq, char * fname);

char * deQ(concurrentQ cq);

int isEmpty(concurrentQ cq);


#endif


