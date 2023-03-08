#include "../include/thread_master.h"
#include "../include/queue.h" 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#include "../include/queue.h"
#include "../include/list.h"
#include "../include/msg.h"
#include "../include/arguments.h"

#include "../include/queue.h"
#include "../include/macro.h"

// Errore DT_DIR su file editor, in compilazione sparisce. 
#define QUIT "QUIT"

extern volatile sig_atomic_t master_running;
extern queue feed_queue;

void    recursive_file_walk_insert(char * dirname, list * l);

/**
 * @brief Funzione del thread master: 
 *      Inserisce nella coda condivisa i file, quando finisce termina se stesso e i thread worker.
 * 
 * @param arg i parametri di configurazione a riga di comando ricevuti precedentemente.  
 * @return void* NULL
 */
void *  master_function (void * arg) {
    
    char *  filename = NULL;
    FarmArguments * config = (FarmArguments * ) arg;

    struct timespec timer; 
    
    //  Nanosleep funziona solo da 0 a 999.999.999 nsec.   
    if (config->farm_setup_delay_time < 1000) {
        timer.tv_sec    = 0;
        timer.tv_nsec   = config->farm_setup_delay_time * MILLION;
    } 
    else {
        timer.tv_sec    = config->farm_setup_delay_time / 1000;
        timer.tv_nsec   = (config->farm_setup_delay_time - timer.tv_sec * 1000) * MILLION;
    }
    
    fprintf(stderr, "Timer msec: %ld\nTimer nanosec: %ld", config->farm_setup_delay_time, timer.tv_nsec);


    while (master_running)
    {
        // Rimuove il primo file sulla lista: 
        filename = list_remove_first(config->farm_setup_file_list);
        
        // Se i files sono terminati esce: 
        if(filename == NULL) break;

        // Accoda un file per i thread worker: 
        queue_enqueue(&feed_queue, filename);

        //Attende lo scadere di "delay" msec: 
        nanosleep(&timer, NULL);

        free(filename);
    }
    
    // Prima di uscire accoda 4 task di terminazione: 
    for (int i = 0; i < config->farm_setup_threads_number; i++) {
        queue_enqueue(&feed_queue, QUIT);
    }

    return NULL;
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

    // Finché accedo ad una cartella ed è tutto ancora attivo:
    // (Potrei ricevere il segnale di terminazione
    //  mentre scorro la lista di files): 

    while ((entry = readdir(dir)) != NULL && master_running) {
        
        // Accodo il percorso ottenuto fino ad ora: 
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        
        if(stat(path, &info) != 1) {
            
            // Se 'path' è una directory: 
            if (S_ISDIR(info.st_mode)) {

                // Ignoro la directory corrente e quella passata: 
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

                // Controllo dentro la directory: 
                recursive_file_walk_insert(path, l);

            } 
            // Se 'path' è un file regolare: 
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

