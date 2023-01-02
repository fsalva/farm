#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    printf("[%s] Mio padre: %d\n", argv[0], getppid());
    exit(0);
}
