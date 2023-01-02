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

bool running = true;

static pthread_mutex_t lock;
static pthread_cond_t cond;

static void * 
thread_function(void *abs_t) {
    struct timespec *abs = (struct timespec *) abs_t;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&lock, NULL);

    pthread_mutex_lock(&lock);
    pthread_cond_timedwait(&cond, &lock, abs);
    running = false;
    pthread_mutex_unlock(&lock);
    return NULL;
}  

int main(int argc, char * const argv[])
{
    printf("[%s] Mio padre: %d\n", argv[0], getppid());
    
    int opt; 

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

     // Create the socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);
    
    pthread_t           timeout_thread;
    

    struct timeval      now;
    struct timespec     time_to_stop;


    gettimeofday(&now, NULL);                               // Prende il tempo 'attuale'
    
    time_to_stop.tv_sec = now.tv_sec + 5;      // Ci aggiunge un timeout specificato da abstime. 
    time_to_stop.tv_nsec = now.tv_usec * 1000;
   
    // Crea un thread che esegue la thread function (attende 5 secondi una lock). 
    // time_to_stop è il parametro della funzione.
    // Alla scadenza setta running = false.
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    
    if(0 == pthread_create(&timeout_thread, &attr, &thread_function, (void *) &time_to_stop)) {
        while(running)
        {   
            // Tentativo di connessione:
            if((connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)){
                
                // Aspetta per "msec" millisecondi, poi riprova.
                usleep(1000);
            }
            else
            {
                // è connesso.
                pthread_cond_signal(&cond);
            //  pthread_join(timeout_thread, NULL);
                break;
            }
            
        }
        
        pthread_attr_destroy(&attr);
        errno = ETIME;
    }




    // Send data to the server and read the response
    char buf[1024] = "Hello, server!";
    write(sockfd, buf, 14);
    sleep(1);
    int n = read(sockfd, buf, 1024);
    printf("Received from server: %s\n", buf);

    // Close the socket
    close(sockfd);

    while ((wpid = wait(&status)) > 0)
    {
        printf("Exit status of %d (Collector) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }




    free(dirname);

    exit(0);
}
