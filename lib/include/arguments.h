#ifndef _ARGUMENTS_FARM_
#define _ARGUMENTS_FARM_

#include "./list.h"

typedef struct FarmArguments {
    int     farm_setup_threads_number;
    int     farm_setup_queue_length;
    int     farm_setup_delay_time;
    char *  farm_setup_directory_name;
    list *  farm_setup_file_list;
} FarmArguments; 

#endif 
