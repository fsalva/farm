#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/list_of_files.h"
#include "../include/file.h"

void list_of_files_init(list_f *l) {
    l->size = 0;
    l->head = NULL;
    l->tail = NULL;
}

int list_of_files_insert_file(list_f *l, file * filename) {

    if(filename == NULL || l == NULL) return -1;

    list_f_node * node = calloc( 1, sizeof(list_f_node) );

    node->value = filename;
    
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


file *  list_of_files_remove_first(list_f *l) {

    if(l->size == 0) {
        return NULL;
    }

    list_f_node * node = l->head;

    l->head = node->next;

    if (l->head == NULL) {
        l->tail = NULL;
    }

    file * value = node->value;
    free(node);

    l->size--;

    return value;
}

int     list_of_files_empty( list_f * l) {
    return l->head == NULL ? 1 : 0;
}

void list_of_files_print(list_f * l) {
    
    list_f_node *node = l->head;
    
    while (node != NULL) {
        fprintf(stdout, "%ld %s\n", node->value->result, node->value->filename);
        node = node->next;
    }
    printf("\n");
}

void list_of_files_destroy(list_f * l)
{  
    list_f_node *element, *nextElement;

    element = l->head;
    
    while(element) {
        nextElement = element->next;
        file_destroy(element->value);
        free(element);
        element = nextElement;
    }
    return;
}