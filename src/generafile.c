#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>       
#include <sys/stat.h>    
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <assert.h>


int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usa: %s nome nelem\n", argv[0]);
    return -1;
  }

  const char *nome = argv[1];
  long nelem = atol(argv[2]);

  if (nelem <0) {
    fprintf(stderr, "nelem non valido\n");
    return -1;
  }
  
  int fd = open(nome, O_CREAT | O_TRUNC | O_RDWR, 0644);
  if (fd == -1) {
    perror("open");
    return -1;
  }

  //unsigned int seed = getpid()*time(NULL); // <-- per generare file sempre diversi 
  unsigned int seed =  331777; 
  if (ftruncate(fd, nelem*sizeof(long)) == -1) {
    perror("ftruncate");
    return -1;
  }
  long *p = mmap(NULL, nelem*sizeof(long), PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0);
  close(fd);
  if (p == MAP_FAILED) {
    perror("mmap");
    return -1;
  }
  long sum=0;
  long *q=p;
  for(long i=0;i<nelem; ++i) {
    *q = (long)(rand_r(&seed) / 12345678.0);
    sum += i * *q;
    ++q;
  }
  munmap(p, nelem*sizeof(long));
  fprintf(stdout, "risultato atteso: %ld\n", sum);
  return 0;
}
