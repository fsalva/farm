#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h> // Socket
#include <sys/types.h>
#include <fcntl.h>  // open
#include <sys/socket.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#include "../include/queue.h"
#include "../include/msg.h"
#include "../include/thread_worker.h"
#include "../include/queue.h"
#include "../include/macro.h"

/**
 * @brief Funzione dei thread:
 *  Crea un server socket e si mette in ascolto di files in arrivo.
 *  Esegue dei calcoli e mantiene una struttura ad albero per garantire
 *  l'ordinamento con una visita in-order. 
 * 
 * @param arg (la coda)
 * @return NULL in uscita. 
 */
void* workers_function(void* arg) {

    int running = 1;

    queue * q = (queue * ) arg;

    // Dimensione del Buffer:  
    //
    // N. Cifre MAX Long su sistemi a 64 bit: 2^64  :   20 + 
    // Dimensisone MAX di path                      :  255 +
    // Carattere di terminazione                    :    1 =
    // ------------------------------------------------------
    //                                                 276  

    char buf[MAX_MSG_SIZE];

    // Crea la socket.
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

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

        // Pulisce il buffer: 
        memset(buf, 0, MAX_MSG_SIZE);

        if(strcmp(filename, QUIT) != 0) {
            sum = sum_longs_from_file(filename);

            if(sum < 0) {
                continue;            
            }
            
            sprintf(buf, "%ld%s", sum, filename);

            int checkv;

            // Se scrivo su socket, e il server è spento: 
            if((checkv = writen(sockfd, buf, MAX_MSG_SIZE)) < 0) {
                running = 0;    // Si spegne
                close(sockfd);
                break; 
            }
            
        }
        else {
            // Se ricevo il task di terminazione:
            // Invio -1 al Collector (+ "QUIT"). 
            sprintf(buf, "%d%s", -1, QUIT);
            writen(sockfd, buf, MAX_MSG_SIZE);
            running = 0;    // Ed esco: 
        }

        free(filename);

    }
    // Chiude la socket.
    close(sockfd);
    
    return NULL;
}




/**
 * @brief Legge 8 bytes (sizeof(long)) alla volta e li somma.
 * 
 * @param filename path del file da elaborare
 * @return long il valore calcolato del file.
 */
long sum_longs_from_file(const char *filename) {

    int fd;
    long number;
    long sum = 0;
    int i = 0;
    ssize_t bytes_read;

    // Apro il file in lettura: 
    fd = open(filename, O_RDONLY); 
    if (fd < 0) {
        return -1;
    }

    // Leggo su variabile long 'number' il contenuto del file:  
    // Finché leggo 8 bytes:
    while ((bytes_read = read(fd, &number, sizeof(long))) == sizeof(long)) {
        sum += number * i;  // Faccio il calcolo
        i++;
    }


    close(fd); // Chiudo il file. 
    return sum;

}