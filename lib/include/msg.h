#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <sys/types.h>

ssize_t  /* Read "n" bytes from a descriptor */
readn(int fd, void *ptr, size_t n);
ssize_t  /* Write "n" bytes to a descriptor */
writen(int fd, void *ptr, size_t n);

#endif