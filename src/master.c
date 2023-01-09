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

#include <pthread.h>
#include <errno.h>

#include "../lib/include/concurrentqueue.h"

#define SOCK_PATH "tmp/farm.sck"  

void *  workers_function( void __attribute((unused)) * arg);
void *  master_function ( void __attribute((unused)) * arg);
void read_files_rec(char * dirname, queue * q);

pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;;


int main(int argc, char * const argv[])
{    
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


    if(delay   == -1 ) delay = 0;

    queue_init(&feed_queue, qlen);


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
        
        read_files_rec(dirname, &feed_queue);

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
        fprintf(stderr, "NEL L'ALTRO LOOP");
    }

            fprintf(stderr, "Connesso..");


    queue * q = (queue * ) arg;

    while (1)  
    {   
        
        char * filename; // -- numero fd socket

        // Acquisisco la lock.
        pthread_mutex_lock(&(q->mutex));
        
        fprintf(stderr, "NEL LOOP");

        //Se non c'è lavoro da fare mi metto in attesa.
        if((filename = queue_dequeue(q)) == NULL){
            
            fprintf(stderr, "Non c'è nulla... mi metto in attesa..");

            // E attendo un segnale per essere risvegliato, rilasciando la lock. 

            pthread_cond_wait(&cond_var, &(q->mutex));

            // Quando viene svegliato deve prendere il primo task disponibile:            
            filename = queue_dequeue(q);

            //fprintf(stderr, "[Thread] Ho ottenuto %s \n", filename);
        } 
        // Mollo la lock sulla coda.

        fprintf(stderr, "[Thread] Ho ottenuto %s \n", filename);



        pthread_mutex_unlock(&(q->mutex)); 

    }
    // Close the socket
    close(sockfd);
    return NULL;
}


void read_files_rec(char * dirname, queue * q) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dirname)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            read_files_rec(path, q);

        } 
        else {
            pthread_mutex_lock(&(q->mutex));            //-- Ottengo la lock sulla coda

            queue_enqueue(q, entry->d_name);

               

            pthread_cond_signal(&(cond_var));    //-- Segnalo ad un thread l'arrivo di un task


            pthread_mutex_unlock(&(q->mutex));
        }
    }

    closedir(dir);
    

}
    


