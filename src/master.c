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

#include <pthread.h>
#include <errno.h>

#define SOCK_PATH "tmp/farm.sck"  

void *              thread_function( void __attribute((unused)) * arg);

int main(int argc, char * const argv[])
{    
    int opt; 

    char * test = malloc(sizeof(char) * 10);

    pthread_t * threadpool;

    // Argomenti opzionali inizializzati con valori default:
    int nthread = 4;
    int qlen = 8;
    int delay = 0; 
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
                fprintf(stderr, "Dirname: %s\n", optarg);
                break;

            case 't':
                delay = atoi(optarg);
                fprintf(stderr, "Delay (msec): %d\n", delay);
                break;

            default:
                fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [-d dirname] [-t time delay]\n", argv[0]);
                break;
        }
    }
    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. Controllo quindi che optind sia inferiore di argc)
    if(optind < argc) {
        while(optind < argc)
            fprintf(stderr, "Argomento non opzionale: %s\n", argv[optind++]);            
    }

    int pid_child = fork();

     // Duplica il processo

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execl("bin/collector", "Collector", NULL);
    }

    int status = 0; int wpid; 

    ///////////////

     // Crea i vari thread. 
    threadpool = malloc(sizeof(pthread_t)* nthread);
 
    for(int i = 0; i < nthread; i++)
    {
        pthread_create(&threadpool[i], NULL, thread_function,NULL);
        
    }
    

    while ((wpid = wait(&status)) > 0)
    {
        printf("Exit status of %d (Collector) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }




    free(dirname);

    exit(0);
}


void * thread_function( void __attribute((unused)) * arg){

    // Create the socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(sockfd == -1) perror("Socket: ");

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    while(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        sleep(0.1);
    }

    while(1) {
        send(sockfd, "Hey!", 4, 0);
        char message[100];

        if(recv(sockfd, message, 100, 0) < 0) {
            perror("Recv: ");
            return NULL;
        }

        fprintf(stderr, "[Worker]: %s\n", message);
        close(sockfd);

        break;
    }

    return NULL;
}