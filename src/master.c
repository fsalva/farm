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
#include "../lib/include/arguments.h"
#include "../lib/include/list.h"

#define SOCK_PATH "tmp/farm.sck"  
#define QUIT "QUIT"
#define MAX_MSG_SIZE 276


void    read_files_rec(char * dirname, queue * q, int delay);
long    sum_longs_from_file(const char *filename);

int     pid_child = -1;
int     running = 1;
int     master_running = 1;


void sigpipe_handler(int signum) {
}

// Invia al processo Collector un SIGUSR2 (Trigger stampa istantanea)
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

    //TODO: Gestione con sigaction
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
    
    // Thread:
    pthread_t   master;
    pthread_t * workers;

    // Argomenti opzionali inizializzati con valori default:
    FarmArguments * config = NULL;

    config = malloc(sizeof(FarmArguments));

    config->farm_setup_threads_number = -1;
    config->farm_setup_delay_time     = -1;
    config->farm_setup_queue_length   = -1;
    config->farm_setup_directory_name = NULL;
    config->farm_setup_file_list = malloc(sizeof(list));
    
    while((opt = getopt(argc, argv, "n:q:d:t:")) != -1) {   //TODO: #3 Controllare il numero degli argomenti, deve essere > 3 se -d non è settata.
        
        switch (opt) {
            case 'n':
                config->farm_setup_threads_number = atoi(optarg);
                break;

            case 'q':
                config->farm_setup_queue_length = atoi(optarg); // TODO: #2 #1  Undefined Behavior se passi una stringa invece di atoi, implementare con strtol > int (check del range se INTEGER).
                break;

            case 'd':
                config->farm_setup_directory_name = malloc(strlen(optarg) + 1);
                strncpy(config->farm_setup_directory_name, optarg, strlen(optarg));
                break;

            case 't':
                config->farm_setup_delay_time = atoi(optarg);
                break;

            default:
                fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [-d config->farm_setup_directory_name] [-t time delay]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /*
    
    
    */
    
    if(config->farm_setup_queue_length == -1 ) 
        config->farm_setup_queue_length = 8;

    // Istanzio coda    
    queue_init(&feed_queue, config->farm_setup_queue_length);
    list_init(config->farm_setup_file_list);

    if(config->farm_setup_delay_time   == -1 ) 
        config->farm_setup_delay_time = 0;

    if(config->farm_setup_directory_name == NULL) {
        if(argc - optind <= 0) {
            fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [filename1, filename2, ..., filenameN] [-t time delay]\n", argv[0]);
            exit(1);
        }
    }
    else {
        // Inserisce i file della directory nella lista di file da elaborare:
        recursive_file_walk_insert(config->farm_setup_directory_name, config->farm_setup_file_list);

    }   

    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. Controllo quindi che optind sia inferiore di argc)
    if(optind < argc) {
        while(optind < argc) {
            // Inserisco gli altri file nella lista di file da elaborare: 
            list_insert(config->farm_setup_file_list, argv[optind++]);            

        }
    }

    int status = 0; int wpid; 

    if(config->farm_setup_threads_number == -1 ) config->farm_setup_threads_number = 4;    

    // Creo Workers thread

    workers = malloc(sizeof(pthread_t) * config->farm_setup_threads_number);




    for(int i = 0; i < config->farm_setup_threads_number; i++) {
        pthread_create(&workers[i], NULL, workers_function, &feed_queue); 
    }
    
    pthread_create(&master, NULL, master_function, config);

    for (int i = 0; i < config->farm_setup_threads_number; i++) {
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

    free(config->farm_setup_directory_name);

    exit(0);
}

