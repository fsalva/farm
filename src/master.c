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

#include "../lib/include/queue.h"
#include "../lib/include/msg.h"

#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276

void *  workers_function( void __attribute((unused)) * arg);
void *  master_function ( void __attribute((unused)) * arg);
void    read_files_rec(char * dirname, queue * q, int delay);
long    sum_longs_from_file(const char *filename);

int pid_child = -1;

void sigpipe_handler(int signum) {
    fprintf(stderr, "Sigpipe ricevuto!\n");
}

void handler_sigusr1(int signum) {
    if(pid_child > 0)
        kill(pid_child, SIGUSR2);
}


int main(int argc, char * const argv[])
{    
    struct sigaction sa; 

    sa.sa_handler = &handler_sigusr1;

    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    signal(SIGPIPE, sigpipe_handler);
    
    pid_child = fork();

     // Duplica il processo

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execl("bin/collector", "Collector", NULL);
    }


    fprintf(stderr, "Master: %d\n", getpid());

    int opt; 
    
    queue feed_queue;

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
        pthread_create(&workers[i], NULL, workers_function, &feed_queue); // TODO: #4 Creare thread master 
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

    while ((wpid = waitpid(pid_child, &status, 0)) > 0)
    {
        printf("Exit status of %d (Collector) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }

    fprintf(stderr, "Sending %d NULL tasks to threads ... \n", nthread);

    for (int i = 0; i < nthread; i++) {
        queue_enqueue(&feed_queue, QUIT);
        sleep(0.1);
    }

    fprintf(stderr, "Waiting for threads ... \n");

    for (int i = 0; i < nthread; i++) {
        pthread_join(workers[i], NULL);

    }

    fprintf(stderr, "Threads joined ! \n");

    free(workers);



    free(dirname);

    exit(0);
}

void* workers_function(void* arg) {

    queue * q = (queue * ) arg;

    int running = 1;

    while (running)  
    {      
        // Create the socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    int mytid = syscall(__NR_gettid);

    // Set up the server address structure
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    // Connect to the server
    while(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        sleep(0.1);
    }

        char * filename; 
        long sum;

        filename = queue_dequeue(q);

        if(strcmp(filename, QUIT) != 0) {
            sum = sum_longs_from_file(filename);
            //fprintf(stderr, "[%ld] Risultato: %ld\n", syscall(__NR_gettid),sum_longs_from_file(filename));
            
            char buf[MAX_MSG_SIZE];
            
            sprintf(buf, "%ld%s", sum, filename);

            int checkv;

            if((checkv = write(sockfd, buf, sizeof(buf))) < 0) {

                running = 0; 
                break; 
            }
            

            read(sockfd, buf, MAX_MSG_SIZE);
        }
        else {
            fprintf(stderr, "Thread %d: received quitting signal! \n", mytid);
            running = 0;

        }

    // Close the socket
        close(sockfd);
        
    
    }

    return NULL;
}


void read_files_rec(char * dirname, queue * q, int delay) {

    DIR *dir;
    struct dirent *entry;
    char path[MAX_MSG_SIZE];

    if (!(dir = opendir(dirname)))
        return;

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            read_files_rec(path, q, delay);

        } 
        else {

            char fullpath[MAX_MSG_SIZE];

            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);
            
            queue_enqueue(q, fullpath);

            usleep(1000 * delay);

            }
            
        }

    closedir(dir);
    

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