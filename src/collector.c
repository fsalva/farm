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

#define SOCK_PATH "tmp/farm.sck"
#define MAXCONN 100

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

#define MAX_MSG_SIZE 276

int print_instantly = 0;

typedef struct file
{
    char * filename;
    long result;

} file;

long compare_elements(file * a, file * b) {
    return a->result > b->result ?  1 : a->result == b->result ? 0 : -1;
}

typedef struct tree
{
    file * f;
    struct tree * left;
    struct tree * right;

} tree;

    tree * t = NULL;


file * createFile(char * filename, long res) {

    file * f = (file *) calloc(1, sizeof(file));

    f->filename = strdup(filename);
    f->result = res;

    return f;
}



tree * addChild(tree * root, file * f) {

    if(root == NULL) {
        
        root = malloc(sizeof(tree));

        root->f= createFile(f->filename, f->result);

        root->left = NULL;
    
        root->right = NULL;

    }
    else {
    
        if(compare_elements(root->f, f) > 0 ){

            root->left = addChild(root->left, f);
        }
        else{ 
            root->right = addChild(root->right, f);
        } 

    }
        
    return root;

}

void printTree(tree * root) {

    if(root != NULL) {
        if(root->left != NULL) printTree(root->left);
        fprintf(stderr, "%ld\t%s\n", (root->f)->result, root->f->filename);
        if(root->right != NULL) printTree(root->right);

    }
    


}

void sig_handler(int signum) {
    
    print_instantly = 1;

}

void int_handler(int signum){

    exit(11);
    
}




static void run_server () {

    int fd_sk, fd_c, max_sockets = 0, fd; 
    
    int nread; 

    char buf[1024];

    fd_set current_sockets, ready_sockets;

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

    

    max_sockets = fd_sk;
    FD_ZERO(&current_sockets);
    FD_SET(fd_sk, &current_sockets);

    int nfd;
    int running = 1;

    while (running) {


        ready_sockets = current_sockets;

        if((nfd = select(max_sockets + 1, &ready_sockets, NULL, NULL, NULL)) < 0) {

            if(errno == EINTR) {
                continue;
            } 
            
            perror("Select: ");
            unlink(SOCK_PATH);
            exit(EXIT_FAILURE);
        }

        else {
            for ( fd = 0; fd < max_sockets + 1; fd++) {
                
                if(print_instantly) {
                    printTree(t);
                    print_instantly = 0;
                }

                if(FD_ISSET(fd, &ready_sockets)) {
                    
                    // Caso 1: Nuova connessione in arrivo: 
                    if(fd == fd_sk) { 
                        
                        fd_c = accept(fd, NULL, 0);

                        FD_SET(fd_c, &current_sockets);

                        if(fd_c > max_sockets) max_sockets = fd_c;
                    }
                    
                    // Caso 2: Pronto in lettura: 
                    else { 
                        
                        // FD_SET(fd_c, &current_sockets);
                        
                        if(readn(fd, buf, MAX_MSG_SIZE) <= 0) {
                            FD_CLR(fd, &current_sockets);
                            close(fd);
                        }

                        // fprintf(stderr, buf);
                        
                        char * restOfTheString;
                        long receivedLong;

                        receivedLong = strtol(buf, &restOfTheString, 10);

                        if(receivedLong <= 0) {
                            running = 0;
                            break;
                        }
                        else {

                            file * f = createFile(restOfTheString, receivedLong);
                            
                            t = addChild(t, f);

                            memset(buf, 0, MAX_MSG_SIZE);

                        }


                    }
                }

            }

        } 
    }

    unlink(SOCK_PATH);
    close(fd_sk);
    fprintf(stderr, "Chiuso socket\n");

}

int main(int argc, char * const argv[])
{

    sigset_t mask;
    
    // Maschera i segnali gestiti da Master-Worker.
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR1);
    
    sigprocmask(SIG_SETMASK, &mask, NULL);

    
    struct sigaction sa; 

    // Imposta l'handler per il segnale USR2 (Inviato da Master-Worker)
    sa.sa_handler = &sig_handler;

    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa, NULL);

    signal(SIGPIPE, int_handler);

        fprintf(stderr, "Collector: %d\n", getpid());


    run_server();

    printTree(t);    

    fprintf(stderr, "Collector: Quitto!\n");
    exit(11);
}
