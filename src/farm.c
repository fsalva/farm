#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>




int main(int argc, char * const argv[])
{

    fprintf(stderr, "MIO PID: %d", getpid());
    execv("bin/master", argv);
}
