#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>

#include "../lib/include/queue.h"
#include "../lib/include/msg.h"
#include "../lib/include/thread_master.h"
#include "../lib/include/thread_worker.h"
#include "../lib/include/arguments.h"
#include "../lib/include/list.h"
#include "../lib/include/macro.h"

int             pid_child = -1;
int             fatal_error = 0;
volatile sig_atomic_t    master_running = 1;


void recursive_file_walk_insert(char * dirname, list * l);


// Invia al processo Collector un SIGUSR2 (Trigger stampa istantanea)
void handler_sigusr1(int signum) {
    (void) signum;
    if(pid_child > 0)
        kill(pid_child, SIGUSR2);
}

void sigint_handler(int signum) {
    (void) signum;
    master_running = 0;
}

void ignore_sigpipe(int signum) { (void) signum; }

void cleanup();

queue feed_queue;

FarmArguments * config = NULL;


int main(int argc, char * const argv[])
{   
    atexit(cleanup);

    //Registra tutti i segnali con rispettivi handlers: 
    struct sigaction sa = {0}; 

    // STAMPA ISTANTANEA (SIGUSR1) : 
    sa.sa_handler = &handler_sigusr1;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    // TERMINAZIONE (INT - QUIT - TERM - HUP): 
    sa.sa_handler = &sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    // Aggiungo anche CHLD
    sigaction(SIGCHLD, &sa, NULL);

    // IGNORA (SIGPIPE)
    sa.sa_handler = &ignore_sigpipe;
    sigaction(SIGPIPE, &sa, NULL);
    
    // Duplica il processo
    pid_child = fork();

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execl("bin/collector", "Collector", NULL);
        _exit(EXIT_FAILURE);
    }
    

    
    // Thread:
    pthread_t   master;
    pthread_t * workers;

    // Argomenti opzionali inizializzati con valori default:
    config = malloc(sizeof(FarmArguments));

    config->farm_setup_threads_number = -1;
    config->farm_setup_delay_time     = -1;
    config->farm_setup_queue_length   = -1;
    config->farm_setup_directory_name = NULL;
    config->farm_setup_file_list = malloc(sizeof(list));

    int opt; 

    while((opt = getopt(argc, argv, "n:q:d:t:")) != -1) {
        
        switch (opt) {
            case 'n':
                config->farm_setup_threads_number = strtol(optarg, NULL, 10);

                if(config->farm_setup_threads_number <= 0 || errno == ERANGE) {

                    PRINT_USAGE_HELP
                    FATAL_ERROR
                    exit(EXIT_FAILURE);
                }

                break;

            case 'q':
                config->farm_setup_queue_length = strtol(optarg, NULL, 10);
                
                if(config->farm_setup_queue_length <= 0 || errno == ERANGE) {

                    PRINT_USAGE_HELP
                    FATAL_ERROR
                    exit(EXIT_FAILURE);
                }

                break;

            case 'd':
                config->farm_setup_directory_name = strdup(optarg);
                if(config->farm_setup_directory_name == NULL){
                    PRINT_USAGE_HELP
                    FATAL_ERROR
                    exit(EXIT_FAILURE);
                }
                break;

            case 't':
                config->farm_setup_delay_time = strtol(optarg, NULL, 10);
                if(config->farm_setup_delay_time < 0 || errno == ERANGE) {

                    PRINT_USAGE_HELP
                    FATAL_ERROR
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                PRINT_USAGE_HELP
                FATAL_ERROR
                exit(EXIT_FAILURE);
        }
    }



    // Non ho cambiato i valori di default della qlen: 
    if(config->farm_setup_queue_length == -1 ) 
        config->farm_setup_queue_length = 8;

    // Istanzio coda dei task:
    queue_init(&feed_queue, config->farm_setup_queue_length);
    
    // Istanzio coda di lettura dei file: 
    list_init(config->farm_setup_file_list);

    // Non ho cambiato i valori di default del delay: 
    if(config->farm_setup_delay_time   == -1 ) 
        config->farm_setup_delay_time = 0;
    // Non ho indicato alcuna directory: 
    if(config->farm_setup_directory_name == NULL) {
        if(argc - optind <= 0) {
            PRINT_USAGE_HELP
            FATAL_ERROR
            exit(EXIT_FAILURE);
        }
    }
    else {
        // Inserisce i file della directory nella lista di file da elaborare:
        recursive_file_walk_insert(config->farm_setup_directory_name, config->farm_setup_file_list);

    }  
    struct stat info = {0};

    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. 
    // Finch?? ho elementi (optind < argc)
    while(optind < argc) {
        // Inserisco gli altri file nella lista di file da elaborare: 
        char * file = strdup(argv[optind++]);

        if(stat(file, &info) != -1) {    
            // Se ho a che fare con un file regolare: 
            if(S_ISREG(info.st_mode)) {
                // Lo inserisco nella lista dei file: 
                list_insert(config->farm_setup_file_list, file);   
            } 
        }
        else { // Gestione errore:
            perror("stat(): ");
        }
        
        free(file);         

    }
    
    // Se non ho indicato il numero di threads: 
    if(config->farm_setup_threads_number == -1 ) 
        config->farm_setup_threads_number = 4;    

    // Creo Threadpool di workers: 
    workers = malloc(sizeof(pthread_t) * config->farm_setup_threads_number);

    for(int i = 0; i < config->farm_setup_threads_number; i++) {
        pthread_create(&workers[i], NULL, workers_function, &feed_queue); 
    }
    
    // Creo master thread: 
    pthread_create(&master, NULL, master_function, config);

    // Attendo la terminazione dei workers: 
    for (int i = 0; i < config->farm_setup_threads_number; i++) {
        pthread_join(workers[i], NULL);

    }

    // Attendo la terminazione del master: 
    pthread_join(master, NULL);

    free(workers);

    exit(EXIT_SUCCESS);
}

void cleanup() {

    int status = 0;  
    int wpid;

    // Faccio pulizia: 
    queue_empty(&feed_queue);
    list_destroy(config->farm_setup_file_list);
    free(config->farm_setup_directory_name);
    free(config->farm_setup_file_list);
    free(config);

    // Se fallisco prima di iniziare l'esecuzione dei thread: 
    if(fatal_error) {
        kill(pid_child, SIGABRT);
    }

    while ((wpid = waitpid(pid_child, &status, 0)) > 0)
    {
    }

}

/**
 * @brief Scorre ricorsivamente tutti i files e tutte le directory (ed il loro contenuto)
 *  A partire dalla cartella dirname. 
 *  Inserisce i nomi dei file trovati nella lista l.
 * 
 * @param dirname 
 * @param l 
 */
void recursive_file_walk_insert(char * dirname, list * l) {

    DIR *dir = {0};
    struct dirent *entry = {0};
    struct stat info = {0};

    char path[MAX_MSG_SIZE];

    // Terminazione della ricorsione: 
    if (!(dir = opendir(dirname)))
        return;

    // Finch?? accedo ad una cartella ed ?? tutto ancora attivo:
    // (Potrei ricevere il segnale di terminazione
    //  mentre scorro la lista di files): 

    while ((entry = readdir(dir)) != NULL && master_running) {
        
        // Accodo il percorso ottenuto fino ad ora: 
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        
        if(stat(path, &info) != 1) {
            
            // Se 'path' ?? una directory: 
            if (S_ISDIR(info.st_mode)) {

                // Ignoro la directory corrente e quella passata: 
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

                // Controllo dentro la directory: 
                recursive_file_walk_insert(path, l);

            } 
            // Se 'path' ?? un file regolare: 
            else if(S_ISREG(info.st_mode))
            {
                // Lo inserisco nella lista di files da elaborare.
                list_insert(l, path);
            }
        }
        else { // Gestione errore
            perror("stat() :");
        }
    }
    closedir(dir);
}
