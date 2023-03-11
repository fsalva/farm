#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/list_of_files.h"

tree * tree_add_node(tree * root, file * f) {

    if(root == NULL) {
        
        root = malloc(sizeof(tree));
        
        root->filelist = malloc(sizeof(list_f));
        
        list_of_files_init(root->filelist);

        list_of_files_insert_file(root->filelist, f);

        root->left = NULL;
    
        root->right = NULL;

    }
    else {
        
        // a > b 
        if(file_compare_elements(root->filelist->head->value, f) > 0) {
            root->left = tree_add_node(root->left, f);
        } // a < b
        else if(file_compare_elements(root->filelist->head->value, f) < 0){ 
            root->right = tree_add_node(root->right, f);
        } // a == b
        else {
            list_of_files_insert_file(root->filelist, f);
            return root;
        }

    }        
    return root;

}

void tree_print(tree * root) {

    if(root != NULL) {
        if(root->left != NULL) {
            tree_print(root->left);
        }

        list_of_files_print(root->filelist);

        if(root->right != NULL){
            tree_print(root->right);
        } 

    }
    
}

void tree_destroy(tree * root){
    if(root != NULL) {
        tree_destroy( root->left );
        tree_destroy( root->right );
        
        list_of_files_destroy(root->filelist);
        free(root->filelist);
        free( root );
        root = NULL;
    }
}