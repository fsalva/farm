#ifndef QUEUE_H
#define QUEUE_H


#include "files.h"


/**
 * @brief Rappresenta una coda di files.
 * 
 */
typedef struct queue
{
    diskfile * file;
    struct queue * next;

} queue;

/***/
int enqueue( queue ** head, char * filename);

void pushQ( queue ** head, int val);

int isEmptyQ( queue * head);

diskfile * dequeue(queue ** head);

#endif
