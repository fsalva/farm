#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    printf("\nHello, test,  my pid is: %d\n", getpid());
    return 0;
}
