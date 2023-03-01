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
#include "../lib/include/thread_master.h"
#include "../lib/include/thread_worker.h"



#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276


void    read_files_rec(char * dirname, queue * q, int delay);
long    sum_longs_from_file(const char *filename);

int pid_child = -1;
int running = 1;
int master_running = 1;


void sigpipe_handler(int signum) {
}

void handler_sigusr1(int signum) {
    if(pid_child > 0)
        kill(pid_child, SIGUSR2);
}

void sigint_handler(int signum) {
    master_running = 0;
}

queue feed_queue;
queue initial_queue;

int main(int argc, char * const argv[])
{    
    struct sigaction sa; 

    sa.sa_handler = &handler_sigusr1;

    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    signal(SIGINT, sigint_handler);

    signal(SIGPIPE, sigpipe_handler);
    

     // Duplica il processo
    pid_child = fork();

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execl("bin/collector", "Collector", NULL);
    }


    fprintf(stderr, "Master: %d\n", getpid());

    int opt; 
    
    

    pthread_t   master;
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

    if(qlen    == -1 ) qlen = 8;

    // Istanzio coda    
    queue_init(&feed_queue, qlen);
    
    // Crea Master Thread



    

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

        //read_files_rec(dirname, &initial_queue, delay);

    }   

    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. Controllo quindi che optind sia inferiore di argc)
    if(optind < argc) {
        while(optind < argc)
            fprintf(stderr, "Argomento non opzionale: %s\n", argv[optind++]);            
    }



    int status = 0; int wpid; 

    ///////////////

   



    if(nthread == -1 ) nthread = 4;    

    // Creo Workers thread

    workers = malloc(sizeof(pthread_t) * nthread);




    for(int i = 0; i < nthread; i++) {
        pthread_create(&workers[i], NULL, workers_function, &feed_queue); 
    }
    
    pthread_create(&master, NULL, master_function, &feed_queue);

    for (int i = 0; i < nthread; i++) {
        pthread_join(workers[i], NULL);

    }
    pthread_join(master, NULL);

    fprintf(stderr, "Ok");


    fprintf(stderr, "Threads joined ! \n");
/*
    while ((wpid = waitpid(pid_child, &status, 0)) > 0)
    {
        printf("Exit status of %d (Collector) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }

*/

    free(workers);

    free(dirname);

    exit(0);
}

