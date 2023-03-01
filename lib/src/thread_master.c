#include "../include/thread_master.h"
#include "../include/queue.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>

#include "../include/queue.h"
#include "../include/msg.h"


#include "../include/queue.h"

#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276

#define QUIT "QUIT"

extern int master_running;

void    read_files_rec(char * dirname, queue * q, int delay);


void *  master_function ( void __attribute((unused)) * arg) {
    
    queue * q = (queue * ) arg;
    char * filename;

    queue_init(q, 8);

    read_files_rec("filetest", q, 1000);

    while (master_running)
    {
        sleep(0.1);

    }
    
    for (int i = 0; i < 4; i++) {
        queue_enqueue(q, QUIT);
    }

    return NULL;
}

void read_files_rec(char * dirname, queue * q, int delay) {

    DIR *dir;
    struct dirent *entry;
    char path[MAX_MSG_SIZE];

    if (!(dir = opendir(dirname)))
        return;

    while ((entry = readdir(dir)) != NULL && master_running) {

        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            read_files_rec(path, q, delay);

        } 
        else {

            char fullpath[MAX_MSG_SIZE];

            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);
            
            usleep(1000 * delay);

            queue_enqueue(q, fullpath);


            }
            
        }

    closedir(dir);
    

}
    
