#ifndef _FILE_FARM_
#define _FILE_FARM_

typedef struct file
{
    char *  filename;
    long    result;
} file;

long    file_compare_elements(file * a, file * b);

file *  file_create(char * filename, long res);
void    file_destroy(file * f);

#endif