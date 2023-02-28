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

void* workers_function(void* arg) {

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

        fprintf(stderr, "Gioco con %s\n", filename);

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
            
            //readn(sockfd, buf, MAX_MSG_SIZE);

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

            queue_enqueue(&feed_queue, fullpath);


            }
            
        }

    closedir(dir);
    

}
    
void *  master_function ( void __attribute((unused)) * arg) {
    
    queue * q = (queue * ) arg;
    char * filename;

    queue_init(q, 8);

    read_files_rec("filetest", q, 100);

    while (master_running)
    {
        sleep(0.1);

    }
    
    for (int i = 0; i < 4; i++) {
        queue_enqueue(q, QUIT);
    }

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