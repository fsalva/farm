#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include "../lib/include/msg.h"
#include "../lib/include/tree.h"
#include "../lib/include/file.h"


#define SOCK_PATH "tmp/farm.sck"
#define MAXCONN 100

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

#define MAX_MSG_SIZE 276

int print_instantly = 0;
int running = 1;

tree * t = NULL;

void sig_handler(int signum) {
    
    print_instantly = 1;

}

void int_handler(int signum){
    running = 0;
}

void abrt_handler(int signum){  // Per test.
    _exit(EXIT_FAILURE);
}

void server_run ();

int main(int argc, char * const argv[])
{
    struct sigaction sa = {0}; 

    sigset_t mask = {0};
    
    // Imposta l'handler per il segnale USR2 (Inviato da Master-Worker)
    sa.sa_handler = &sig_handler;

    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = &abrt_handler;
    sigaction(SIGABRT, &sa, NULL);

    
    // Maschera i segnali gestiti da Master-Worker.
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGPIPE);

    sigprocmask(SIG_SETMASK, &mask, NULL);

    server_run();

    tree_print(t);    

    tree_destroy(t);

    
    exit(EXIT_SUCCESS);
}


void server_run () {

    int fd; 
    int fd_c; 
    int fd_sk; 
    int max_socket = 0;             // Ottimizza ciclo select
    int current_sockets_number = 0; // Controlla se ci sono socket client ancora attive. 

    char buf[MAX_MSG_SIZE];

    fd_set current_sockets;
    fd_set ready_sockets;

    struct sockaddr_un psa;
    memset(&psa, 0, sizeof(psa));
    psa.sun_family = AF_UNIX;
    strcpy(psa.sun_path, SOCK_PATH);



    if((fd_sk = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket: "); 
        exit(EXIT_FAILURE);
    }

    if(bind(fd_sk, (struct sockaddr *) &psa, sizeof(psa)) < 0) { 
        perror("Bind: "); 
        exit(EXIT_FAILURE);
    }

    if(listen(fd_sk, MAXCONN) < 0) {
        perror("Listen: ");
        exit(EXIT_FAILURE);
    }

    

    max_socket = fd_sk;
    FD_ZERO(&current_sockets);
    FD_SET(fd_sk, &current_sockets);

    int nfd;

    while (running) {

        ready_sockets = current_sockets;

        if((nfd = select(max_socket + 1, &ready_sockets, NULL, NULL, NULL)) < 0) {

            if(errno == EINTR) {
                continue;
            } 
            
            perror("Select: ");
            unlink(SOCK_PATH);
            exit(EXIT_FAILURE);
        }

        else {

            for ( fd = 0; fd < max_socket + 1; fd++) {
                
                if(print_instantly) {
                    tree_print(t);
                    print_instantly = 0;
                }

                if(FD_ISSET(fd, &ready_sockets)) {
                    
                    // Caso 1: Nuova connessione in arrivo: 
                    if(fd == fd_sk) { 
                        
                        fd_c = accept(fd, NULL, 0);

                        current_sockets_number++;

                        FD_SET(fd_c, &current_sockets);

                        if(fd_c > max_socket) max_socket = fd_c;
                    }
                    
                    // Caso 2: Pronto in lettura: 
                    else { 
                                                
                        if(readn(fd, buf, MAX_MSG_SIZE) <= 0) {
                            FD_CLR(fd, &current_sockets);
                            close(fd);
                        }
                        
                        char *  restOfTheString;
                        long    receivedLong;

                        receivedLong = strtol(buf, &restOfTheString, 10);

                        if(receivedLong <= 0) { // Uno dei thread worker si è spento.
                            current_sockets_number--;
                            FD_CLR(fd, &current_sockets);
                            close(fd);
                            
                            if(current_sockets_number == 0) {
                                running = 0;    // Se non ho più fd attivi smetto di funzionare.
                            } 
                            
                            break;
                        }
                        else {
                            
                            // Creo il file: 
                            file * f = file_create(restOfTheString, receivedLong);
                            
                            // Lo aggiungo ad un albero binario: 
                            t = tree_add_node(t, f);

                            memset(buf, 0, MAX_MSG_SIZE);

                        }
                    }
                }
            }
        } 
    }

    unlink(SOCK_PATH);
    close(fd_sk);
}