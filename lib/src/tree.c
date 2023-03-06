#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/file.h"

tree * addChild(tree * root, file * f) {


    if(root == NULL) {
        
        root = malloc(sizeof(tree));

        root->f = f;

        root->left = NULL;
    
        root->right = NULL;

    }
    else {
    
        if(compare_elements(root->f, f) > 0 ){

            root->left = addChild(root->left, f);
        }
        else if(compare_elements(root->f, f) < 0){ 
            root->right = addChild(root->right, f);
        } 

        else {
            free(f->filename);
            free(f);
        }

    }        
    return root;

}

void printTree(tree * root) {

    if(root != NULL) {
        if(root->left != NULL) {
            printTree(root->left);
        }
        fprintf(stdout, "%ld %s\n", (root->f)->result, root->f->filename);

        if(root->right != NULL){
            printTree(root->right);
        } 

    }
    
}

void treeprint(tree *root, int level)
{
        if (root == NULL)
                return;
        for (int i = 0; i < level; i++)
                printf(i == level - 1 ? "|-" : "  ");
        printf("[%d] :%ld\n", level, root->f->result);
        treeprint(root->left, level + 1);
        treeprint(root->right, level + 1);
}

void postOrderFree(tree * root){
    if(root != NULL) {
        postOrderFree( root->left );
        postOrderFree( root->right );
        
        destroy_file(root->f);

        free( root );
        root = NULL;
    }
}