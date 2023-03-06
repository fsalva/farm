#ifndef _TREE_FARM_
#define _TREE_FARM_

#include "./file.h"

typedef struct tree
{
    file * f;
    struct tree * left;
    struct tree * right;

} tree;


tree * tree_add_node(tree * root, file * f);

void tree_print(tree * root);

void tree_destroy(tree * root);


#endif