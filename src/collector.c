#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>


#define SOCK_PATH "tmp/farm.sck"
#define MAXCONN 100

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"



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
        printTree(root->left);
        fprintf(stderr, "%ld\t%s\n", (root->f)->result, root->f->filename);
        printTree(root->right);

    }
    


}



void sig_handler(int signum){

    printf("\nInside handler function\n");
    
    printTree(t);

    fflush(stdout);
    unlink(SOCK_PATH);
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


    fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    unlink(SOCK_PATH);
    if(fd_sk < 0) {
        perror("Socket:");
    }

    int check4err = bind( fd_sk, (struct sockaddr *) &psa, sizeof(psa));

    if (check4err < 0) { perror("Bind: "); }

    listen(fd_sk, MAXCONN);

    

    max_sockets = fd_sk;
    FD_ZERO(&current_sockets);
    FD_SET(fd_sk, &current_sockets);

    while (1) {


        ready_sockets = current_sockets;

        if( select(max_sockets + 1, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("Select: ");
        
            unlink(SOCK_PATH);
            exit(EXIT_FAILURE);
        }


        else {
            for ( fd = 0; fd < max_sockets + 1; fd++) {


                if(FD_ISSET(fd, &ready_sockets)) {
                    
                    // Caso 1: Nuova connessione in arrivo: 
                    if(fd == fd_sk) { 
                        
                        fd_c = accept(fd, NULL, 0);

                        FD_SET(fd_c, &current_sockets);

                        if(fd_c > max_sockets) max_sockets = fd_c;
                    }
                    
                    // Caso 2: Pronto in lettura: 
                    else { 

                        read(fd, buf, 1024);

                        //fprintf(stderr, buf);
                        
                        char * restOfTheString;
                        long receivedLong;

                        receivedLong = strtol(buf, &restOfTheString, 10);

                        file * f = createFile(restOfTheString, receivedLong);
                        
                        t = addChild(t, f);

                        //fprintf(stderr, ">>>>>>[%ld][%s]\n", receivedLong, restOfTheString);


                        //createFile(restOfTheString, receivedLong);
                        
                        write(fd, "Ok", sizeof("Ok"));

                        FD_CLR(fd, &current_sockets);


                        close(fd);

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

    signal(SIGINT, sig_handler);

    run_server();
    
    

    exit(11);
}
