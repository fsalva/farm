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


#include "../include/thread_worker.h"
#include "../include/queue.h"

#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276

extern int master_running;

void* workers_function(void* arg) {

    int running = 1;
    queue * q = (queue * ) arg;

    char buf[MAX_MSG_SIZE];

    // Crea la socket.
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    int mytid = syscall(__NR_gettid);

    // Set-up del server.
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    // Si connette - (Aspetta se il Collector non è pronto). 
    while(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        sleep(0.1);
    }

    while (running)  
    {          
        char * filename; 
        long sum;

        filename = queue_dequeue(q);

        memset(buf, 0, MAX_MSG_SIZE);

        if(strcmp(filename, QUIT) != 0) {
            sum = sum_longs_from_file(filename);            
            
            sprintf(buf, "%ld%s", sum, filename);

            int checkv;

            if((checkv = writen(sockfd, buf, MAX_MSG_SIZE)) < 0) {

                running = 0; 
                close(sockfd);
                break; 
            }
            
        }
        else {
            sprintf(buf, "%d%s", -1, QUIT);
            writen(sockfd, buf, MAX_MSG_SIZE);
            running = 0;
        }

    }

    fprintf(stderr, "Thread %d: quitting! \n", mytid);

    // Chiude la socket.
    close(sockfd);
    
    return NULL;
}





long sum_longs_from_file(const char *filename) {

    int fd;
    long number;
    long sum = 0;
    int i = 0;
    ssize_t bytes_read;

    fd = open(filename, O_RDONLY); // open the file for reading
    if (fd < 0) {
        printf("Error opening file!\n");
        return -1;
    }

    while ((bytes_read = read(fd, &number, sizeof(long))) == sizeof(long)) {
        sum += number * i;
        i++;
    }


    close(fd); // close the file
    return sum;

}