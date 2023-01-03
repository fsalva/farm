#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define SOCK_PATH "tmp/farm.sck"
#define MAXCONN 100

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

void* handle_client(void* arg) {
    // Convert the void* pointer to an int
    int client_sockfd = *((int*)arg);

    // Read data from the client socket and write it back
    char buf[1024];
    int n = read(client_sockfd, buf, 1024);
    fprintf(stderr, "[Server] %d \n", n);
    write(client_sockfd, buf, n);

    // Close the client socket
    close(client_sockfd);
    return NULL;
}

static void run_server () {
    int fd_sk, fd_c, max_sockets = 0, fd; 
    int nread; 

    char buf[1024];

    fd_set set, rdset;

    struct sockaddr_un psa;
    memset(&psa, 0, sizeof(psa));
    psa.sun_family = AF_UNIX;
    strcpy(psa.sun_path, SOCK_PATH);


    fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(fd_sk < 0) {
        perror("Socket:");
    }

    int check4err = bind( fd_sk, (struct sockaddr *) &psa, sizeof(psa));

    if (check4err < 0) { perror("Bind: "); }

    listen(fd_sk, MAXCONN);

    fprintf(stderr, "[Server] Creata socket, pronta! \n");



    FD_ZERO(&set);
    FD_SET(fd_sk, &set);

    max_sockets = fd_sk;

    while (1) {

        rdset = set;

        if( select(max_sockets + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Select: ");
            break;
        }
        else {
            for ( fd = 0; fd <= max_sockets + 1; fd++) {

                if(FD_ISSET(fd, &rdset)) {
                    
                    if(fd == fd_sk) { // Accetta connessione
                        fd_c = accept(fd, NULL, 0);

                        fprintf(stderr, "[Server] Accettato una connessione. \n");

                        FD_SET(fd_c, &set);

                        if(fd_c > max_sockets) max_sockets = fd_c;
                    }
                    else { // Pronto in lettura: 
                        
                        fprintf(stderr, "Client pronto in lettura\n");

                        //OPERAZIONI
                        handle_client((void *) &fd_c);

                        
                        FD_CLR(fd, &set);
                    }
                }
            }

        } 
    }

    close(fd_sk);
    fprintf(stderr, "Chiuso socket\n");

}

int main(int argc, char * const argv[])
{

    run_server();

    // Select
    fd_set rdset;

    FD_ZERO(&rdset);

    // Close the client socket

    // Close the socket when finished
    
    unlink(SOCK_PATH);

    printf("[%s] Mio padre: %d\n", argv[0], getppid());
    exit(11);
}
