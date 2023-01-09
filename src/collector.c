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

typedef struct {
    int res;
    char * filename;

    fileAndResult * next;

} fileAndResult;



int compare_elements(fileAndResult * a, fileAndResult * b) {
    return a->res > b->res ?  1 : a->res == b->res ? 0 : -1;
}

void insert_sorted( fileAndResult * list, size_t len) {

    for (size_t i = 1; i < len; i++) {
        
        fileAndResult temp = list[i];
        
        size_t j = i;
        
        while (j > 0 && compare_elements(&temp, &list[j - 1]) < 0) {
            list[j] = list[j - 1];
            j--;
        }
        
        list[j] = temp;
  }
}

static void run_server () {
    int fd_sk, fd_c, max_sockets = 0, fd; 
    int nread; 

    fileAndResult * list;
    int llen = 0;

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

        FD_ZERO(&rdset);

        rdset = set;

        if( select(max_sockets + 1, &rdset, NULL, NULL, NULL) == -1) {
            continue;
        }
        else {
            for ( fd = 0; fd <= max_sockets + 1; fd++) {

                if(FD_ISSET(fd, &rdset)) {
                    
                    if(fd == fd_sk) { // Accetta connessione
                        
                        fd_c = accept(fd, NULL, 0);

                        FD_SET(fd_c, &set);

                        if(fd_c > max_sockets) max_sockets = fd_c;
                    }
                    else { // Pronto in lettura: 
                        
                        //OPERAZIONI
                        int n = read(fd, buf, 1024);

                        char * restOfTheString = NULL;
                        long receivedLong = strtol(buf, &restOfTheString, 10);
                        
                        restOfTheString = strtok(strdup(restOfTheString), "\t");

                        fprintf(stderr, "[%ld][%s]\n", receivedLong, restOfTheString);


                        close(fd);
                        
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

    exit(11);
}
