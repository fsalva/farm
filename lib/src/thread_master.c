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
    

    while (master_running)
    {
        // Rimuove il primo file sulla lista: 
        filename = list_remove_first(config->farm_setup_file_list);
        
        // Se i files sono terminati esce: 
        if(filename == NULL) break;

        // Accoda un file per i thread worker: 
        queue_enqueue(&feed_queue, filename);

        errno = 0;
        //Attende lo scadere di "delay" msec:
        // Se nanosleep viene interrotta da un signal, 
        // usando il flag SA_RESTART la funzione riparte, 
        // ma controllo che sia stata interrotta proprio con errno: EINTR.
        while (nanosleep(&timer, &timer) && errno == EINTR);

        if(errno != 0 && errno != EINTR) { 
            free(filename);
            break;
        }

        free(filename);
    }
    
    // Prima di uscire accoda 4 task di terminazione: 
    for (int i = 0; i < config->farm_setup_threads_number; i++) {
        queue_enqueue(&feed_queue, QUIT);
    }

    return NULL;
}


