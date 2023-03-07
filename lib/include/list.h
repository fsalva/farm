#ifndef _FARM_LIST_H_
#define _FARM_LIST_H_

typedef struct list_node {
    char *value; 
    struct list_node *next;
} list_node;

/**
 * @brief Rappresenta una lista di files.
 * 
 */
typedef struct list {
    
    list_node *head;
    list_node *tail;

    int size;

} list;


void    list_init(list *l);
int     list_insert(list *l, char * filename);
char *  list_remove_first(list *l);
int     list_empty( list * l);
void    list_print(list * l);
void    list_destroy(list * l);


#endif
