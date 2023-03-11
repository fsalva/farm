#ifndef _FARM_LIST_FILES_H_
#define _FARM_LIST_FILES_H_

#include "./file.h"

typedef struct list_f_node {
    file * value; 
    struct list_f_node *next;
} list_f_node;

/**
 * @brief Rappresenta una lista di files.
 * 
 */
typedef struct list_f {
    
    list_f_node *head;
    list_f_node *tail;

    int size;

} list_f;


void    list_of_files_init(list_f * l);
int     list_of_files_insert_file(list_f *l, file * filename);
file *  list_of_files_remove_first(list_f * l);
int     list_of_files_empty( list_f * l);
void    list_of_files_print(list_f * l);
void    list_of_files_destroy(list_f * l);


#endif
