
#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "../include/queue.h"


void queue_init(queue *q) {
    q->head = NULL;
    q->tail = NULL;
}

// Enqueues a new element at the end of the queue
int queue_enqueue(queue *q, char *value) {

    queue_node *node = calloc( 1, sizeof(queue_node));

    node->value = calloc(strlen(value) + 1, sizeof(char));
    if(!node->value) return -1;

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

    return 0;
}

char *queue_dequeue(queue *q) {
    if (q->head == NULL) {
        // The queue is empty
        return NULL;
    }

    queue_node *node = q->head;
    q->head = node->next;
    if (q->head == NULL) {
        // The queue is now empty
        q->tail = NULL;
    }

    char *value = node->value;
    free(node);
    return value;
}


int isEmptyQ(queue * head) {
    return head == NULL ? 1 : 0;
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
