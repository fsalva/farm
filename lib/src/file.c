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

    f->filename = strdup(filename);
    f->result = res;

    return f;
}


#endif
