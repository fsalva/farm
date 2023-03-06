#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h> // Socket
#include <sys/types.h>
#include <fcntl.h>  // open
#include <sys/socket.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#include "../include/queue.h"
#include "../include/msg.h"
#include "../include/thread_worker.h"
#include "../include/queue.h"
#include "../include/macro.h"

void* workers_function(void* arg) {

    int running = 1;
    queue * q = (queue * ) arg;

    char buf[MAX_MSG_SIZE];

    // Crea la socket.
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

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

            if(sum < 0) {
                continue;            
            }
            
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

        free(filename);

    }
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
        return -1;
    }

    while ((bytes_read = read(fd, &number, sizeof(long))) == sizeof(long)) {
        sum += number * i;
        i++;
    }


    close(fd); // close the file
    return sum;

}