#ifndef MYFILE_H
#define MYFILE_H

#include "../include/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long compare_elements(file * a, file * b) {
    return a->result > b->result ?  1 : a->result == b->result ? 0 : -1;
}

file * createFile(char * filename, long res) {

    file * f = (file *) calloc(1, sizeof(file));

    f->filename = ( char * ) calloc(strlen(filename) + 1, sizeof(char));
    strncpy(f->filename, filename, strlen(filename));
    f->result = res;

    return f;
}

void destroy_file(file * f) {
    
    free(f->filename);

    free(f);
}


#endif
