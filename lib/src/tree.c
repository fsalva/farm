#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"

tree * addChild(tree * root, file * f) {

    if(root == NULL) {
        
        root = malloc(sizeof(tree));

        root->f= createFile(f->filename, f->result);

        root->left = NULL;
    
        root->right = NULL;

    }
    else {
    
        if(compare_elements(root->f, f) > 0 ){

            root->left = addChild(root->left, f);
        }
        else{ 
            root->right = addChild(root->right, f);
        } 

    }
        
    return root;

}

void printTree(tree * root) {

    if(root != NULL) {
        if(root->left != NULL) printTree(root->left);
        fprintf(stderr, "%ld\t%s\n", (root->f)->result, root->f->filename);
        if(root->right != NULL) printTree(root->right);

    }
    


}

