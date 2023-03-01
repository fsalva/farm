#ifndef _FILE_FARM_
#define _FILE_FARM_

typedef struct file
{
    char * filename;
    long result;

} file;

long compare_elements(file * a, file * b);

file * createFile(char * filename, long res);

#endif