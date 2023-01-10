
#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "../include/queue.h"


void queue_init(queue *q, int capacity) {

    q->capacity = capacity;
    q->size = 0;

    pthread_mutex_init(&(q->mutex), NULL);

    pthread_cond_init(&(q->nemptyCond), NULL);
    pthread_cond_init(&(q->nfullCond), NULL);

    q->head = NULL;
    q->tail = NULL;
}

// Enqueues a new element at the end of the queue
int queue_enqueue(queue *q, char *value) {

    pthread_mutex_lock(&q->mutex);

    queue_node *node = calloc( 1, sizeof(queue_node));

    

    node->value = calloc(strlen(value) + 1, sizeof(char));
    if(!node->value) return -1;

    while(q->capacity - q->size == 0) {
        pthread_cond_wait(&q->nfullCond, &q->mutex);
    }

    node->value = strdup(value);
    node->next = NULL;

    if (q->tail == NULL) {
        // The queue is empty
        q->head = node;
        q->tail = node;
    } else {
        // Add the element to the end of the queue
        q->tail->next = node;
        q->tail = node;
    }


    q->size++;

    
    pthread_cond_signal(&q->nemptyCond);

    pthread_mutex_unlock(&q->mutex);

    return 0;
}

char *queue_dequeue(queue *q) {

    pthread_mutex_lock(&(q->mutex));



    while(q->size == 0) {
        pthread_cond_wait(&q->nemptyCond, &q->mutex);
    }


    queue_node *node = q->head;
    q->head = node->next;
    if (q->head == NULL) {
        // The queue is now empty
        q->tail = NULL;
    }

    char *value = node->value;
    free(node);

    q->size--;

    pthread_cond_signal(&(q->nfullCond));
    
    pthread_mutex_unlock(&(q->mutex));


    return value;
}


int isEmptyQ(queue * head) {

    //pthread_mutex_lock(&(head->mutex));

    int retval = head == NULL ? 1 : 0;

    //pthread_mutex_unlock(&(head->mutex));


    return retval;
}

void queue_print(queue *q) {
    queue_node *node = q->head;
    while (node != NULL) {
        printf("%s ", node->value);
        node = node->next;
    }
    printf("\n");
}

#endif
