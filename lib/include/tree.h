#ifndef _TREE_FARM_
#define _TREE_FARM_

#include "./file.h"

typedef struct tree
{
    file * f;
    struct tree * left;
    struct tree * right;

} tree;


tree * addNode(file * f);

tree * addChild(tree * root, file * f);

tree * addLeft(tree * root, file * f);

tree * addRight(tree * root, file * f); 

void printTree(tree * root);


#endif