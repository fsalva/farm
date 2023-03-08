#ifndef _ARGUMENTS_FARM_
#define _ARGUMENTS_FARM_

#include <pthread.h>

#include "./list.h"

typedef struct FarmArguments {
    long    farm_setup_threads_number;
    long    farm_setup_queue_length;
    long    farm_setup_delay_time;
    char *  farm_setup_directory_name;
    list *  farm_setup_file_list;
    
    pthread_cond_t * farm_setup_termination_cond;
} FarmArguments; 

#endif 
