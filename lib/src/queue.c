#include <stdlib.h>
#include <stdio.h>
#include "../include/queue.h"

void print_queue(node_t ** head){
   node_t * curr = * head;

    while (curr != NULL)
    {
        fprintf(stderr, "%d", curr->val);
        curr = curr->next;
    }
}

void enqueue(node_t ** head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t ** head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

int isEmptyQ(node_t * head) {
    return head == NULL ? 1 : 0;
}


//insert link at the first location 
void pushQ(node_t ** head, int val) {
   //create a link
    node_t *link = ( node_t*) malloc(sizeof(node_t));
	//
    link->val = val;
	
   //point it to old first elem
    link->next = * head;
	
   //point first to new first elem
   * head = link;

}