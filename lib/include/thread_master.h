#ifndef _THREAD_MASTER_C
#define _THREAD_MASTER_C

#include "./list.h"

void *  master_function ( void __attribute((unused)) * arg);
void    recursive_file_walk_insert(char * dirname, list * l);

#endif