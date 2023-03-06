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
#include "../include/macro.h"

// Errore DT_DIR su file editor, in compilazione sparisce. 
#define _GNU_SOURCE

#define QUIT "QUIT"

extern int master_running;
extern queue feed_queue;

void    recursive_file_walk_insert(char * dirname, list * l);

void *  master_function ( void __attribute((unused)) * arg) {
    
    char *  filename = NULL;
    FarmArguments * config = (FarmArguments * ) arg;

    while (master_running)
    {
        filename = list_remove_first(config->farm_setup_file_list);
        
        if(filename == NULL) break;

        queue_enqueue(&feed_queue, filename);
        usleep(config->farm_setup_delay_time * 1000);

        free(filename);
    }
    
    for (int i = 0; i < config->farm_setup_threads_number; i++) {
        queue_enqueue(&feed_queue, QUIT);
    }

    return NULL;
}

void recursive_file_walk_insert(char * dirname, list * l) {

    DIR *dir = {0};
    struct dirent *entry = {0};
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

            }
            
        }

    closedir(dir);
    

}
    
