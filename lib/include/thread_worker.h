#ifndef _THREAD_WORKER_C
#define _THREAD_WORKER_C


long    sum_longs_from_file(const char *filename);

void *  workers_function(void* arg);

#endif