
#ifndef MYFILES_H
#define MYFILES_H

typedef struct diskfile{

    char * path; 

    long result; 

} diskfile;

void write_files_recursively(char *filename, char * dirname, long nfiles);

#endif