#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>




int main(int argc, char * const argv[])
{

    

    // Duplica il processo
    int pid_child = fork();

    // Se sei il figlio, fai partire il processo Master-Worker
    if (pid_child == 0) {
        execv("bin/master", argv);
    }
    
    int status = 0; int wpid; 

    while ((wpid = waitpid( pid_child, &status, 0)) > 0)
    {
        printf("Exit status of %d (Master-Worker) was %d (%s)\n", (int)wpid, WEXITSTATUS(status),
            (status > 0) ? "accept" : "reject");
    }


    return 0;
}
