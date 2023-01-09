#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <limits.h>

#include <pthread.h>
#include <errno.h>

#include "../lib/include/concurrentqueue.h"

#define SOCK_PATH "tmp/farm.sck"  

void *  workers_function( void __attribute((unused)) * arg);
void *  master_function ( void __attribute((unused)) * arg);
void    read_files_rec(char * dirname, queue * q, int delay);
long    sum_longs_from_file(const char *filename);


int main(int argc, char * const argv[])
{    
    int pid_child = fork();

     // Duplica il processo

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execl("bin/collector", "Collector", NULL);
    }

    int opt; 
    
    queue feed_queue;

    char * test = malloc(sizeof(char) * 10);

    pthread_t * workers;

    // Argomenti opzionali inizializzati con valori default:
    int nthread = -1;
    int qlen = -1;
    int delay = -1; 
    char * dirname = NULL;


    while((opt = getopt(argc, argv, "n:q:d:t:")) != -1) {   //TODO: #3 Controllare il numero degli argomenti, deve essere > 3 se -d non è settata.
        switch (opt) {
            case 'n':
                nthread = atoi(optarg);
                fprintf(stderr, "N.Threads: %d\n",  nthread);
                break;

            case 'q':
                qlen = atoi(optarg); // TODO: #2 #1  Undefined Behavior se passi una stringa invece di atoi, implementare con strtol > int (check del range se INTEGER).
                fprintf(stderr, "Queue length: %d\n", qlen);
                break;

            case 'd':
                dirname = malloc(strlen(optarg) + 1);
                strncpy(dirname, optarg, strlen(optarg));
                    
                break;

            case 't':
                delay = atoi(optarg);
                fprintf(stderr, "Delay (msec): %d\n", delay);
                break;

            default:
                fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [-d dirname] [-t time delay]\n", argv[0]);
                exit(1);
        }
    }

    if(nthread == -1 ) nthread = 4;

    // Creo Workers thread


    workers = malloc(sizeof(pthread_t) * nthread);
 
    for(int i = 0; i < nthread; i++) {
        pthread_create(&workers[i], NULL, workers_function, &feed_queue);
    }

    if(qlen    == -1 ) qlen = 8;

    // Istanzio coda
    queue_init(&feed_queue, qlen);


    if(delay   == -1 ) delay = 0;





    if(dirname == NULL) {
        if(argc - optind <= 0) {
            fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [filename1, filename2, ..., filenameN] [-t time delay]\n", argv[0]);
            exit(1);
        }

        else {
            // Gestione altri file

        } 
    }
    else {

        read_files_rec(dirname, &feed_queue, delay);

    } 

    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. Controllo quindi che optind sia inferiore di argc)
    if(optind < argc) {
        while(optind < argc)
            fprintf(stderr, "Argomento non opzionale: %s\n", argv[optind++]);            
    }


    

    int status = 0; int wpid; 

    ///////////////


    for (int i = 0; i < nthread; i++) {
        pthread_join(workers[i], NULL);
    }


    while ((wpid = wait(&status)) > 0)
    {
        printf("Exit status of %d (Collector) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }




    free(dirname);

    exit(0);
}

void* workers_function(void* arg) {

    FILE * f;

    // Create the socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    int mytid = syscall(__NR_gettid);

    // Set up the server address structure
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    // Connect to the server
    while(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        continue;
    }

    queue * q = (queue * ) arg;

    while (1)  
    {   
        char * filename; 
        int sum;

        filename = queue_dequeue(q);

        sum = sum_longs_from_file(filename);
        fprintf(stderr, "[%ld] Risultato: %ld\n", syscall(__NR_gettid),sum_longs_from_file(filename));
        
        char buf[1024];

        snprintf(buf, sizeof(buf), "%d\t%s", sum, filename);

        write(sockfd, buf, sizeof(buf));

    }
    // Close the socket
    close(sockfd);

    return NULL;
}


void read_files_rec(char * dirname, queue * q, int delay) {

    DIR *dir;
    struct dirent *entry;
    char path[1024];
    char parentD[1024];

    if (!(dir = opendir(dirname)))
        return;

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            read_files_rec(path, q, delay);

        } 
        else {

            char fullpath[1024];

            snprintf(fullpath, sizeof(fullpath), "%s\/%s", dirname, entry->d_name);
            
            queue_enqueue(q, fullpath);

            usleep(1000 * delay);

            //fprintf(stderr, "\r                          ");
            }
            
        }

    closedir(dir);
    

}
    


long sum_longs_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");

    //fprintf(stderr, "Filename: %s\n", filename);

    if (fp == NULL) {
        perror("Error opening file");
        return 0;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    long sum = 0;
    errno = 0;
    int index = 0;

    while ((read = getline(&line, &len, fp)) != -1) {
        char *endptr;
        long value = strtol(line, &endptr, 10);
        if (errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)) {
        perror("Error parsing long value");
        break;
        }
        if (line == endptr) {
        fprintf(stderr, "No digits found in line\n");
        } else {
        sum += (value * index++);
        }
    }
    free(line);
    fclose(fp);

    return sum;
}