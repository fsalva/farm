#include <stdio.h>
#include <unistd.h>
#include "../lib/include/macro.h"

int main(int argc, char * const argv[])
{   
    if(argc == 1) {
        PRINT_USAGE_HELP
        return -1;
    }    


    execv("bin/masterworker", argv);

    // Errore di execv
    return -1;
}
