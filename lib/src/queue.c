
#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "../include/queue.h"

// Inizializzo la coda, le due condition variables e la mutex: 
void queue_init(queue *q, int capacity) {

    q->capacity = capacity;
    q->size = 0;

    pthread_mutex_init(&(q->mutex), NULL);

    pthread_cond_init(&(q->nemptyCond), NULL);
    pthread_cond_init(&(q->nfullCond), NULL);

    q->head = NULL;
    q->tail = NULL;
}

// Inserisco un nuovo elemento nella coda
// Se e solo se ho spazio disponibile
// (altrimenti attendo che si liberi spazio)
int queue_enqueue(queue *q, char *value) {

    pthread_mutex_lock(&q->mutex);

    queue_node *node = calloc( 1, sizeof(queue_node));

    node->value = calloc(strlen(value) + 1, sizeof(char));
    if(!node->value) return -1;

    // Attendo che si liberi spazio: 
    while(q->capacity - q->size == 0) {
        pthread_cond_wait(&q->nfullCond, &q->mutex);
    }

    strncpy(node->value, value, strlen(value));
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

    // Ho appena inserito un elemento, segnalo che la lista non è vuota,
    // (Chi è in attesa di prelevare elementi viene svegliato)
    pthread_cond_signal(&q->nemptyCond);

    pthread_mutex_unlock(&q->mutex);

    return 0;
}

// Prelevo un elemento dalla coda
// Solo se la coda non è vuota.
// (altrimenti attendo che si riempia di almeno un elemento)
char * queue_dequeue(queue *q) {

    pthread_mutex_lock(&(q->mutex));

    // Attendo che la coda si popoli: 
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

    // Ho tolto un elemento, vuol dire che la 
    // coda sicuramente non è vuota.
    // (Permette di eseguire la enqueue())
    pthread_cond_signal(&(q->nfullCond));
    
    pthread_mutex_unlock(&(q->mutex));


    return value;
}

void queue_empty(queue *q) {
    
    queue_node *element, *nextElement;

    element = q->head;
    
    while(element) {
        nextElement = element->next;
        free(element);
        element = nextElement;
    }
}

#endif
