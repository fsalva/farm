#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define SOCK_PATH "tmp/farm.sck"
#define SERVER_BACKLOG 100

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

int main(int argc, char * const argv[])
{

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) perror("Socket:");



    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;

    strcpy(server_addr.sun_path, SOCK_PATH);

    int rc = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rc < 0 ) {
        perror("Bind:");
    }



    listen(sockfd, SERVER_BACKLOG);

    fprintf(stderr, "[Server] Creata socket, pronta! \n");


    int clientsock = accept(sockfd, NULL, NULL);

     char buf[1024];
    int n = read(clientsock, buf, 1024);
    fprintf(stderr, "[Server] %d \n", n);
    write(clientsock, buf, n);

    // Close the client socket
    close(clientsock);


        

    // Close the socket when finished
    close(sockfd);

    unlink(SOCK_PATH);

    printf("[%s] Mio padre: %d\n", argv[0], getppid());
    exit(11);
}
