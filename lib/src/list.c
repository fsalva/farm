#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/list.h"

void list_init(list *l) {

    l->size = 0;
    l->head = NULL;
    l->tail = NULL;
}

int list_insert(list *l, char * filename) {

    list_node * node = calloc( 1, sizeof(list_node));

    node->value = calloc(strlen(filename) + 1, sizeof(char));
    
    if(node->value == NULL) return -1;

    strncpy(node->value, filename, strlen(filename));
    node->next = NULL;

    if (l->tail == NULL) {
        l->head = node;
        l->tail = node;
    } else {
        l->tail->next = node;
        l->tail = node;
    }

    l->size++;

    return 0;
}


char *  list_remove_first(list *l) {

    if(l->size == 0) {
        return NULL;
    }

    list_node * node = l->head;

    l->head = node->next;

    if (l->head == NULL) {
        l->tail = NULL;
    }

    char *value = node->value;
    free(node);

    l->size--;

    return value;
}

int     list_empty( list * l) {
    return l->head == NULL ? 1 : 0;
}

void    list_print(list * l) {
    
    list_node *node = l->head;
    
    while (node != NULL) {
        printf("%s ", node->value);
        node = node->next;
    }
    printf("\n");
}

void list_destroy(list * l)
{  
    list_node *element, *nextElement;

    element = l->head;
    
    while(element) {
        nextElement = element->next;
        free(element);
        element = nextElement;
    }
    return;
}