#ifndef _FARM_QUEUE_H
#define _FARM_QUEUE_H

#include <pthread.h>

typedef struct queue_node {
    char *value;  // Pointer to the character array
    struct queue_node *next;
} queue_node;

/**
 * @brief Rappresenta una coda di files.
 * 
 */
typedef struct queue {
    queue_node *head;
    queue_node *tail;

    int size;
    int capacity;

    pthread_mutex_t mutex;
    pthread_cond_t nemptyCond;
    pthread_cond_t nfullCond;

} queue;


/***/

void queue_init(queue *q, int qlen);

int queue_enqueue(queue *q, char *value);

char * queue_dequeue(queue *q);

void queue_print(queue * q);

void queue_empty(queue *q);


#endif
