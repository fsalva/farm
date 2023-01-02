#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>

int main(int argc, char * const argv[])
{
    int opt; 

    // Argomenti opzionali inizializzati con valori default:
    int nthread = 4;
    int qlen = 8;
    int delay = 0; 
    char * dirname = NULL;

    while((opt = getopt(argc, argv, "n:q:d:t:")) != -1) {
        switch (opt) {
            case 'n':
                nthread = atoi(optarg);
                fprintf(stderr, "N.Threads: %d\n",  nthread);
                break;

            case 'q':
                qlen = atoi(optarg); // TODO: #2 #1  Undefined Behavior se passi una stringa invece di atoi, implementare con strtol > int (check del range se INTEGER).
                fprintf(stderr, "Queue length: %d\n", qlen);
                break;

            case 'd':
                dirname = malloc(strlen(optarg) + 1);
                strncpy(dirname, optarg, strlen(optarg));
                fprintf(stderr, "Dirname: %s\n", optarg);
                break;
            case 't':
                delay = atoi(optarg);
                fprintf(stderr, "Delay (msec): %d\n", delay);
                break;
            
            default:
                fprintf(stderr, "Usage: %s [-n nthread] [-q queue length] [-d dirname] [-t time delay]\n", argv[0]);
                break;
        }
    }
    // Gestione argomenti obbligatori (getopt() li ordina e li inserisce in coda. Controllo quindi che optind sia inferiore di argc)
    if(optind < argc) {
        while(optind < argc)
            fprintf(stderr, "Argomento non opzionale: %s\n", argv[optind++]);
            
    }
    
    free(dirname);

    return 0;
}
