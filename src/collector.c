#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * const argv[])
{
    printf("[%s] Mio padre: %d\n", argv[0], getppid());
    exit(11);
}
