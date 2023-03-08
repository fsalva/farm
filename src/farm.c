#include <stdio.h>
#include <unistd.h>

int main(int argc, char * const argv[])
{   
    (void) argc;
    execv("bin/masterworker", argv);

    // Errore di execv
    return -1;
}
