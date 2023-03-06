#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/file.h"

tree * tree_add_node(tree * root, file * f) {

    if(root == NULL) {
        
        root = malloc(sizeof(tree));

        root->f = f;

        root->left = NULL;
    
        root->right = NULL;

    }
    else {
    
        if(file_compare_elements(root->f, f) > 0 ){

            root->left = tree_add_node(root->left, f);
        }
        else if(file_compare_elements(root->f, f) < 0){ 
            root->right = tree_add_node(root->right, f);
        } 

        else {
            free(f->filename);
            free(f);
        }

    }        
    return root;

}

void tree_print(tree * root) {

    if(root != NULL) {
        if(root->left != NULL) {
            tree_print(root->left);
        }
        fprintf(stdout, "%ld %s\n", (root->f)->result, root->f->filename);

        if(root->right != NULL){
            tree_print(root->right);
        } 

    }
    
}

void tree_destroy(tree * root){
    if(root != NULL) {
        tree_destroy( root->left );
        tree_destroy( root->right );
        
        file_destroy(root->f);

        free( root );
        root = NULL;
    }
}