
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/queue.h"


int enqueue(queue ** head, char * filename) {

    queue * new_node = malloc(sizeof(queue));
    if (!new_node) return -1;

    new_node->file->path =  malloc(sizeof(char) * strlen(filename));

    strncpy(new_node->file->path, filename, strlen(filename));
    new_node->next = *head;

    *head = new_node;

    return 0;
}

diskfile * dequeue(queue ** head) {

    queue * current, * prev = NULL;
    
    diskfile * retval = NULL;

    if (* head == NULL) return NULL;

    current = * head;

    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval->path = malloc(sizeof(char) * strlen(current->file->path));
    strncpy(retval->path, current->file->path, strlen(current->file->path)); 
    
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

int isEmptyQ(queue * head) {
    return head == NULL ? 1 : 0;
}

