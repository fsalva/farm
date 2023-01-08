#ifndef QUEUE_H
#define QUEUE_H



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
} queue;


/***/

void queue_init(queue *q);

int queue_enqueue(queue *q, char *value);

int isEmptyQ( queue * head);

char * queue_dequeue(queue *q);

void queue_print(queue * q);


#endif
