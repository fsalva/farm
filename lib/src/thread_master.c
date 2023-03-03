#include "../include/thread_master.h"
#include "../include/queue.h" 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>

#include "../include/queue.h"
#include "../include/list.h"
#include "../include/msg.h"
#include "../include/arguments.h"

#include "../include/queue.h"

#define _GNU_SOURCE

#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276

#define QUIT "QUIT"

extern int master_running;
extern queue feed_queue;

void    recursive_file_walk_insert(char * dirname, list * l);

void *  master_function ( void __attribute((unused)) * arg) {
    
    FarmArguments * config = (FarmArguments * ) arg;
    char *  filename = NULL;

    while (master_running)
    {
        filename = list_remove_first(config->farm_setup_file_list);
        
        if(filename == NULL) break;

        queue_enqueue(&feed_queue, filename);
        usleep(config->farm_setup_delay_time * 1000);
    }
    
    for (int i = 0; i < config->farm_setup_threads_number; i++) {
        queue_enqueue(&feed_queue, QUIT);
    }

    return NULL;
}

void recursive_file_walk_insert(char * dirname, list * l) {

    DIR *dir;
    struct dirent *entry;
    char path[MAX_MSG_SIZE];

    if (!(dir = opendir(dirname)))
        return;

    while ((entry = readdir(dir)) != NULL && master_running) {

        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            recursive_file_walk_insert(path, l);

        } 
        else {

            char fullpath[MAX_MSG_SIZE];

            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);

            list_insert(l, fullpath);
            //fprintf(stderr, "Inserito: %s\n", fullpath);

            }
            
        }

    closedir(dir);
    

}
    
