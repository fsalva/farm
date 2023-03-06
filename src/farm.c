#include <stdio.h>
#include <unistd.h>




int main(int argc, char * const argv[])
{
    execv("bin/master", argv);
}
